#include "csapp.h"
#include "semaphore.h"
#include <stdlib.h>
#include <stdio.h>
#include "client_registry.h"
#include "debug.h"
#include "server.h"

int max = 3000;

typedef struct CLIENT_NODE {
    int fd;
    struct CLIENT_NODE* next;
} CLIENT_NODE;

typedef struct client_registry {
    sem_t mutex;
    sem_t empty;
    CLIENT_NODE* clients; //linked list to store the clients because it dynamically expands, easier than array

} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
  CLIENT_REGISTRY *client = malloc(sizeof(struct client_registry)); 
  if(client == NULL) return NULL; //error case

  client->clients = NULL; //initialize the fields of the client registry
  sem_init(&client->mutex, 0, 1);
  sem_init(&client->empty, 0, 0); //initial value of 0 to indicate locked

  return client;
}

void creg_fini(CLIENT_REGISTRY *cr){
  sem_destroy(&cr->mutex);

  CLIENT_NODE* curr = cr->clients; //current ptr points to linked list of client registry
    while (curr != NULL) { //loop thru the clients list and free each one
        CLIENT_NODE* next = curr->next; 
        free(curr);
        curr = next;
    }

    free(cr);
  
}

int creg_register(CLIENT_REGISTRY *cr, int fd){ //adding new client to clientlist
    CLIENT_NODE* node = malloc(sizeof(CLIENT_NODE)); //create the new node
    if(node == NULL) return -1; //error check

    sem_wait(&cr->mutex); //wait for semaphore to become available

    node->fd = fd; //assign the fields of the new node 
    node->next = NULL;

    if(cr->clients == NULL){ //if the linked list is empty, set the head of linklist equal to the new node
        cr->clients = node;
    } 
    else{
        CLIENT_NODE* curr = cr->clients;
        while(curr->next != NULL){ //need to do curr.next in order to actually append to end of link list
            curr = curr->next;
        }
        curr->next = node;
    }

    sem_post(&cr->mutex); //post after semaphore is finished waiting and unblocked

    return 0;
}

int creg_unregister(CLIENT_REGISTRY *cr, int fd){
    //remove client from clientlist
    CLIENT_NODE* curr = cr->clients;
    CLIENT_NODE* prev = NULL;
    sem_wait(&cr->mutex);

    if(curr == NULL){
        sem_post(&cr->mutex);
        return -1; //the head of Linklist is null, client registry not set yet
    }

    if(curr->fd == fd){ //if the head is the client to remove
        cr->clients =  curr->next;
        free(curr); //delete the head
        

        if(cr->clients == NULL){ //link list is now empty after removing 
            sem_post(&cr->empty);
        }

        sem_post(&cr->mutex);
        return 0;

    }

    while(curr != NULL){
        if(curr->fd == fd){
            prev->next = curr->next; //set the prev equal to currs next, removing it from linklist
            free(curr); //delete client node

            if(cr->clients == NULL){ //link list is now empty after removing 
            sem_post(&cr->empty);
             }

             sem_post(&cr->mutex);
            return 0;

        }
        prev = curr;
        curr = curr->next;
    }

    sem_post(&cr->mutex);
    return -1; //error if the client was not found in the list
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    for(;;){
        sem_wait(&cr->mutex);
        
        if (cr->clients == NULL) {
            sem_post(&cr->mutex);
            return;

        }

        sem_post(&cr->mutex);

    }
    
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    //shutdown all socket connections for registered clients

    sem_wait(&cr->mutex);

    CLIENT_NODE* curr = cr->clients;
    while(curr != NULL){
        shutdown(curr->fd, SHUT_RDWR);
        curr = curr->next;
    }

    sem_post(&cr->mutex);
    
}
