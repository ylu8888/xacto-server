#include "server.h"
#include "csapp.h"
#include <pthread.h>
#include "data.h"
#include "protocol.h"
#include "client_registry.h"
#include "transaction.h"

extern CLIENT_REGISTRY *client_registry;

void *xacto_client_service(void *arg){
	int connfd = *((int *)arg);
	Pthread_detach(pthread_self());
	Free(arg);

	creg_register(client_registry, connfd);

	TRANSACTION *trans = trans_create();

	if(trans == NULL){
	     Close(connfd);
	     return NULL;
	}

	for(;;){
	     XACTO_PACKET *reqpkt = malloc(sizeof(XACTO_PACKET)); //request packet
	     void *datap = NULL;
	     void *datak = NULL;
	     void *datav = NULL;

	     //this gets the serial num, stored in datap
	     if(proto_recv_packet(connfd, &reqpkt, &datap) == -1){
		break; //error
	     }

	     if(reqpkt->type == XACTO_PUT_PKT){

		    //this gets the KEY, stored in datak
		     if(proto_recv_packet(connfd, &reqpkt, &datak) == -1){ 
			break; //error
		     }

		     //this gets the VALUE, stored in datav
		     if(proto_recv_packet(connfd, &reqpkt, &datav) == -1){ 
			break; //error
		     }

		     TRANS_STATUS tstat = store_put(trans, (KEY*) datak, (BLOB*) datav); //put a key/value mapping in store

		     //use proto_send_pkt for REPLY after key and value
		     XACTO_PACKET *reppkt = malloc(sizeof(XACTO_PACKET)); //make a xacto packet with type reply
		     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
		     reppkt->status = tstat; //the output of STORE is a status, send that in as a reppkt->status
		     reppkt->serial = (ntohl)(uint32_t)datap; //serial number is gotten from the first read
		     reppkt->null = 0;  //rest is 0
		     reppkt->size = 0;
		     reppkt->timestamp_sec = 0;
		     reppkt->timestamp_nsec = 0;

		     void* datas = NULL;
		     if(proto_send_packet(connfd, &reppkt, datas) == -1){ //send after making REPLY packet
			break; //error
		     }
		    
	     }
	     else if(reqpkt->type == XACTO_GET_PKT){
		     //XACTO_KEY_PKT

		     //use proto_send_pkt for REPLY after key and value
		     
		   
	     }
	     else if(reqpkt->type == XACTO_COMMIT_PKT){

		     
	     }

	     
	     
		
	}

	free(reqpkt);
	creg_unregister(client_registry, 0);
	trans_unref(tp, "Terminating client service thread");
	
	Close(connfd);
	return NULL;
}
