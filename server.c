#include "server.h"
#include "csapp.h"
#include <pthread.h>
#include "data.h"
#include "protocol.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "debug.h"

CLIENT_REGISTRY *client_registry;

// void *xacto_client_service(void *arg){
//  int connfd = *((int *)arg);
//  Pthread_detach(pthread_self());
//  Free(arg);
//  Close(connfd);
//  return NULL;
// }

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

	void *datap; //reads the SERIAL #
    void *datak; //reads the KEY
    void *datav; //reads the VALUE

	for(;;){

		XACTO_PACKET *reqpkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //request packet
		XACTO_PACKET *reppkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //reply packet
		XACTO_PACKET *datapkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //data packet

		memset(reqpkt, 0, sizeof(XACTO_PACKET));
		memset(reppkt, 0, sizeof(XACTO_PACKET));
		memset(datapkt, 0, sizeof(XACTO_PACKET));

		// XACTO_PACKET *reqpkt = Calloc(1, sizeof(XACTO_PACKET)); //request packet
		// XACTO_PACKET *reppkt = Calloc(1, sizeof(XACTO_PACKET)); //reply packet
		// XACTO_PACKET *datapkt = Calloc(1, sizeof(XACTO_PACKET)); //data packet

	     datap = NULL; //reads the SERIAL #
	     datak = NULL; //reads the KEY
	     datav = NULL; //reads the VALUE

	     //this gets the serial num, stored in datap
	     if(proto_recv_packet(connfd, reqpkt, &datap) == -1) break;
	     
	     if(reqpkt->type == XACTO_PUT_PKT){
		    //this gets the KEY, stored in datak
		     if(proto_recv_packet(connfd, reqpkt, &datak) == -1) break;

		     //this gets the VALUE, stored in datav
		     if(proto_recv_packet(connfd, reqpkt, &datav) == -1) break;

		  // create blob with a size(ntohl) and key store ptr
		     BLOB* blobVal = blob_create(datak, ntohl(reqpkt->size));
		     KEY* tempKey = key_create(blobVal);
		  // key ptr = key create(just created blob)

		     //CREATE another blob with the value store ptr
		     BLOB* blobVal2 = blob_create(datav, ntohl(reqpkt->size)); //BLOBVAL2 IS FOR THE VALUE

		     TRANS_STATUS tstat = store_put(trans, tempKey, blobVal2); //put a key/value mapping in store

		     //use proto_send_pkt for REPLY after key and value
		     //make a xacto packet with type reply
		     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
		     reppkt->status = tstat; //the output of STORE is a status, send that in as a reppkt->status
		     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
		     reppkt->null = 0;  //rest is 0
		     reppkt->size = 0; //the size is 0 because we're not sending any payloads
		     reppkt->timestamp_sec = 0;
		     reppkt->timestamp_nsec = 0;

		     //void* datas = NULL;
		     if(proto_send_packet(connfd, reppkt, NULL) == -1) break; //send after making REPLY packet

		     if(tstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }
		     
		      //key_dispose(tempKey);
		      //blob_unref(blobVal2, "DISPOSAL OF BLOB");

	     }
	     else if(reqpkt->type == XACTO_GET_PKT){
		     if(proto_recv_packet(connfd, reqpkt, &datak) == -1)break;   //this gets the KEY, stored in datak
		     
		     BLOB* blobVal = blob_create(datak, ntohl(reqpkt->size)); //blobVal is a BLOB for getting the KEY
		     KEY* tempKey = key_create(blobVal); //creating a key
		     
		     BLOB* valBlob = NULL; //valBlob is a BLOB for getting the VALUE
		     TRANS_STATUS gstat = store_get(trans, tempKey, &valBlob);  //the value from store_get is stored inside of valBlob

		     BLOB* newVal = blob_create(valBlob->content, strlen(valBlob->content));
		     
		     //use proto_send_pkt for REPLY after key and value
		     //make a xacto packet with type reply
		     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
		     reppkt->status = gstat; //the output of STORE is a status, send that in as a reppkt->status
		     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
		     reppkt->null = 0;  //rest is 0
		     reppkt->size = 0;
		     reppkt->timestamp_sec = 0;
		     reppkt->timestamp_nsec = 0;

		     //THIS IS THE DATA PACKET, ABOVE IS THE REPLY PACKET

		     datapkt->type = XACTO_VALUE_PKT;
		     datapkt->status = gstat; 
		     datapkt->serial = reqpkt->serial;
		     if(valBlob->content == NULL){
			datapkt->null = 1;
			datapkt->size = 0;
		     } else{
			datapkt->null = 0;
			datapkt->size = ntohl(strlen(valBlob->content));
		     }
		     
		     datapkt->timestamp_sec = 0;
		     datapkt->timestamp_nsec = 0;
		     
		    //WE SEND IN TWO PACKETS: REPLY AND DATA PCKET
		     //for the third argument void* data: send in NULL for the reply and value Blob for the data
		     //we want to send in the VALUE from GET in the packet, which is stored in newVal

		     //  void* datax = NULL; 
		     if(proto_send_packet(connfd, reppkt, NULL) == -1)break; //send after making REPLY packet
		     
		     //  if(proto_send_packet(connfd, datapkt, valBlob->content) == -1)
		     if(proto_send_packet(connfd, datapkt, newVal->content) == -1)break; //send after making REPLY packet

		     if(gstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }
		      //key_dispose(tempKey);
		     // blob_unref(newVal, "DISPOSAL OF BLOB");
		     
	     }
	     else if(reqpkt->type == XACTO_COMMIT_PKT){
	     TRANS_STATUS cstat = trans_commit(trans);

	    //make a xacto packet with type reply
	     reppkt->type = XACTO_REPLY_PKT; //initialize the xacto packet struct for REPLY
	     reppkt->status = cstat; //the output of STORE is a status, send that in as a reppkt->status
	     reppkt->serial = reqpkt->serial; //serial number is gotten from the first read
	     reppkt->null = 0;  //rest is 0
	     reppkt->size = 0;
	     reppkt->timestamp_sec = 0;
	     reppkt->timestamp_nsec = 0;

	     if(proto_send_packet(connfd, reppkt, NULL) == -1){ //send after making REPLY packet
		break; //error
	     }

		if(cstat == TRANS_ABORTED){
		   trans_abort(trans); //effects of an aborted trans are removed from the 
		   break;
		}


		break; //once we commit, we're done we want to break out of the infinite while loop
	     }

		//this is the end of the while loop
		Free(reqpkt);
		Free(datapkt);
		Free(reppkt);
		Free(datap);
		Free(datak);
		Free(datav);

	}

	creg_unregister(client_registry, connfd);
	//trans_unref(trans, "Terminating client service thread");
	
	Close(connfd);
	return NULL;
}
