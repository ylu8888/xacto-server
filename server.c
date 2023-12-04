#include "server.h"
#include "csapp.h"
#include <pthread.h>
#include "data.h"
#include "protocol.h"
#include "client_registry.h"
#include "transaction.h"

void *xacto_client_service(void *arg){
	int connfd = *((int *)arg);
	Pthread_detach(pthread_self());
	Free(arg);

	CLIENT_REGISTRY *cr;

	creg_register(cr, 0);

	TRANSACTION *trans = trans_create();

	if(trans == NULL){
	     Close(connfd);
	     return NULL;
	}

	for(;;){
	     XACTO_PACKET req; //request packet
		
	}

	creg_unregister(cr, 0);
	trans_unref(tp, "Terminating client service thread");
	
	Close(connfd);
	return NULL;
}
