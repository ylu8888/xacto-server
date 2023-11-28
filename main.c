#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include <stdio.h>
#include <stdlib.h>

static void terminate(int status);

CLIENT_REGISTRY *client_registry;

void sig_handler(int signum){
   //implement it for when you catch a SIGHUP

   //kill func to terminate
}

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

  struct sigaction sigact;
  sigact.sa_handler = sig_handler; //assign the signal handler
  sigact.sa_flags = SA_RESTART;
  sigaction(SIGHUP, &sa, NULL);
	
 if(sigaction(SIGCHLD, &sigact, NULL) == -1){ //IF ENCOUNTER AN ERROR
 }
	
 int listenfd, *connfdp;
 socklen_t clientlen;
 struct sockaddr_storage clientaddr;
 pthread_t tid;
 listenfd = Open_listenfd(argv[2]); //arg2 to get the port number
	
 for(;;){ //infinite while loop 
	clientlen=sizeof(struct sockaddr_storage);
	connfdp = Malloc(sizeof(int));
	*connfdp = Accept(listenfd,
	 (SA *) &clientaddr, &clientlen);
	Pthread_create(&tid, NULL, xacto_client_service, connfdp);
 }
    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
