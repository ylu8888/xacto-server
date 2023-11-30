#include "csapp.h"
#include "client_registry.h"
#include "semaphore.h"

int max = 3000;

typedef struct client_registry {
    sem_t mutex;
    CLIENT_NODE* clients; //linked list to store the clients because it dynamically expands, easier than array

} CLIENT_REGISTRY;

typedef struct client_node {
    int fd;
    struct CLIENT_NODE* next;
} CLIENT_NODE;

CLIENT_REGISTRY *creg_init(){
  CLIENT_REGISTRY client = Malloc(sizeof(CLIENT_REGISTRY)); 
  if(client == NULL) return NULL; //error case

  cr->clients = NULL; //initialize the fields of the client registry
  sem_init(&cr->mutex, 0, 1);

  return cr;
}

void creg_fini(CLIENT_REGISTRY *cr){
  sem_destroy(&cr->mutex);

  CLIENT_NODE* curr = cr->clients; //current ptr points to linked list of client registry
    while (current != NULL) { //loop thru the clients list and free each one
        CLIENT_NODE* next = curr->next; 
        free(curr);
        curr = next;
    }

    free(cr);
  
}

int creg_register(CLIENT_REGISTRY *cr, int fd){
    CLIENT_NODE* node = malloc(sizeof(CLIENT_NODE)); //create the new node
    if(node == NULL) return -1; //error check

    sem_wait(&cr->mutex);

    node->fd = fd; //assign the fields of the new node 
    node->next = NULL;

    if(cr->clients == NULL){ //if the linked list is empty, set the head of linklist equal to the new node
        cr->clients = node;
    } 
    else{
        CLIENT_NODE* curr = cr->clients;
        while(curr.next != NULL){ //need to do curr.next in order to actually append to end of link list
            curr = curr.next;
        }
        curr.next = node;
    }

    sem_post(&cr->mutex);

    return 0;
}
