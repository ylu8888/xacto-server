#include "server.h"
#include "csapp.h"
#include <pthread.h>
#include "data.h"
#include "protocol.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "debug.h"

extern CLIENT_REGISTRY *client_registry;

void *xacto_client_service(void *arg){
	int connfd = *((int *)arg);
	Pthread_detach(pthread_self());
	Free(arg);

	creg_register(client_registry, connfd);

	TRANSACTION *trans = trans_create();

	XACTO_PACKET *reqpkt;

	if(trans == NULL){
	     Close(connfd);
	     return NULL;
	}

	for(;;){
	     reqpkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //request packet
	     void *datap = NULL; //reads the SERIAL #
	     void *datak = NULL; //reads the KEY
	     void *datav = NULL; //reads the VALUE

	     //this gets the serial num, stored in datap
	     if(proto_recv_packet(connfd, reqpkt, &datap) == -1){
		break; //error
	     }

	     if(reqpkt->type == XACTO_PUT_PKT){

		    //this gets the KEY, stored in datak
		     if(proto_recv_packet(connfd, reqpkt, &datak) == -1){ 
			break; //error
		     }

		     //this gets the VALUE, stored in datav
		     if(proto_recv_packet(connfd, reqpkt, &datav) == -1){ 
			break; //error
		     }

		  // create blob with a size of recieved->size (ntohl) and key_store ptr
		     BLOB* blobVal = blob_create(datak, ntohl(reqpkt->size));
		     KEY* tempKey = key_create(blobVal);
		  // key ptr = key create(just created blob)

		     TRANS_STATUS tstat = store_put(trans, tempKey, blobVal); //put a key/value mapping in store

		     //use proto_send_pkt for REPLY after key and value
		     XACTO_PACKET *reppkt = malloc(sizeof(XACTO_PACKET)); //make a xacto packet with type reply
		     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
		     reppkt->status = tstat; //the output of STORE is a status, send that in as a reppkt->status
		     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
		     reppkt->null = 1;  //rest is 0
		     reppkt->size = 0; //the size is 0 because we're not sending any payloads
		     reppkt->timestamp_sec = 0;
		     reppkt->timestamp_nsec = 0;

		     void* datas = NULL;
		     if(proto_send_packet(connfd, reppkt, datas) == -1){ //send after making REPLY packet
			break; //error
		     }

		     if(tstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }
		    
	     }
	     else if(reqpkt->type == XACTO_GET_PKT){
		     //XACTO_KEY_PKT

		     //this gets the KEY, stored in datak
		     if(proto_recv_packet(connfd, reqpkt, &datak) == -1){ 
			break; //error
		     }

		     BLOB* valBlob = NULL; //stores thre value

		     //blobVal is a BLOB for getting the KEY
		     //valBlob is a BLOB for getting the VALUE
		     BLOB* blobVal = blob_create(datak, ntohl(reqpkt->size)); //blob with keyptr 
		     KEY* tempKey = key_create(blobVal); //creating a key
		     
		     //the value from store_get is stored inside of datav, as per the third argument &datav.
		     TRANS_STATUS gstat = store_get(trans, tempKey, &valBlob); //put a key/value mapping in store

		     BLOB* newVal = blob_create(valBlob->content, sizeof(*valBlob));
		     
		     //use proto_send_pkt for REPLY after key and value
		     XACTO_PACKET *reppkt = malloc(sizeof(XACTO_PACKET)); //make a xacto packet with type reply
		     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
		     reppkt->status = gstat; //the output of STORE is a status, send that in as a reppkt->status
		     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
		     reppkt->null = 0;  //rest is 0
		     reppkt->size = 0;
		     reppkt->timestamp_sec = 0;
		     reppkt->timestamp_nsec = 0;

		     //THIS IS THE DATA PACKET, ABOVE IS THE REPLY PACKET
		     XACTO_PACKET *datapkt = malloc(sizeof(XACTO_PACKET)); 
		     datapkt->type = XACTO_VALUE_PKT;
		     datapkt->status = gstat; 
		     datapkt->serial = reqpkt->serial;
		     if(valBlob->content == NULL){
			datapkt->null = 1;
		     } else{
			datapkt->null = 0;
		     }
		     datapkt->size = sizeof(*valBlob);
		     datapkt->timestamp_sec = 0;
		     datapkt->timestamp_nsec = 0;
		     
		    //WE SEND IN TWO PACKETS: REPLY AND DATA PCKET
		     //for the third argument void* data: send in NULL for the reply and value Blob for the data
		     void* datax = NULL;
		     //we want to send in the VALUE from GET in the packet, which is stored in DATAV
		     if(proto_send_packet(connfd, reppkt, datax) == -1){ //send after making REPLY packet
			break; //error
		     }

		     if(proto_send_packet(connfd, datapkt, valBlob) == -1){ //send after making REPLY packet
			break; //error
		     }

		     if(gstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }
		     
		   
	     }
	     else if(reqpkt->type == XACTO_COMMIT_PKT){
	     TRANS_STATUS cstat = trans_commit(trans);

	     XACTO_PACKET *reppkt = malloc(sizeof(XACTO_PACKET)); //make a xacto packet with type reply
	     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
	     reppkt->status = cstat; //the output of STORE is a status, send that in as a reppkt->status
	     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
	     reppkt->null = 0;  //rest is 0
	     reppkt->size = 0;
	     reppkt->timestamp_sec = 0;
	     reppkt->timestamp_nsec = 0;

	     void* dataz = NULL;
	     if(proto_send_packet(connfd, reppkt, dataz) == -1){ //send after making REPLY packet
		break; //error
	     }

		if(cstat == TRANS_ABORTED){
		   trans_abort(trans); //effects of an aborted trans are removed from the 
		   break;
		}

		break; //once we commit, we're done we want to break out of the infinite while loop
	     }

		
	}

	free(reqpkt);
	creg_unregister(client_registry, 0);
	trans_unref(trans, "Terminating client service thread");
	
	Close(connfd);
	return NULL;
}
