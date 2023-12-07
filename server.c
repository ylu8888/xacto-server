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

		XACTO_PACKET *reqpkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //request packet
		XACTO_PACKET *reppkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //reply packet
		XACTO_PACKET *datapkt = (XACTO_PACKET *)Malloc(sizeof(XACTO_PACKET)); //data packet

		// memset(reqpkt, 0, sizeof(XACTO_PACKET));
		// memset(reppkt, 0, sizeof(XACTO_PACKET));
		// memset(datapkt, 0, sizeof(XACTO_PACKET));
	     
	     void *datap = NULL; //reads the SERIAL #
	     void *datak = NULL; //reads the KEY
	     void *datav = NULL; //reads the VALUE

	     //this gets the serial num, stored in datap
	     proto_recv_packet(connfd, reqpkt, &datap);
	     
	     if(reqpkt->type == XACTO_PUT_PKT){
		    //this gets the KEY, stored in datak
		     proto_recv_packet(connfd, reqpkt, &datak);

		     //this gets the VALUE, stored in datav
		     proto_recv_packet(connfd, reqpkt, &datav);

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

		     void* datas = NULL;
		     proto_send_packet(connfd, reppkt, datas); //send after making REPLY packet

		     if(tstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }
		     
		     // key_dispose(tempKey);
		     // blob_unref(blobVal2);

	     }
	     else if(reqpkt->type == XACTO_GET_PKT){
		     proto_recv_packet(connfd, reqpkt, &datak);   //this gets the KEY, stored in datak
		     
		     BLOB* blobVal = blob_create(datak, ntohl(reqpkt->size)); //blobVal is a BLOB for getting the KEY
		     KEY* tempKey = key_create(blobVal); //creating a key
		     
		     debug("before storeget");
		     debug("%s", tempKey->blob->content);
		     
		     BLOB* valBlob = NULL; //valBlob is a BLOB for getting the VALUE
		     TRANS_STATUS gstat = store_get(trans, tempKey, &valBlob);  //the value from store_get is stored inside of valBlob
		     
		     debug("after storeget");
		     
		     if(valBlob == NULL){
		     	debug("I AM NULLLL");
		     	break;
		     } else{
		     	debug("I AM NOT NULLLL");
		     	debug("%s", valBlob->content);
		        debug("%zu", sizeof(*valBlob));
		     }

		     BLOB* newVal = blob_create(valBlob->content, sizeof(*valBlob));

		     //debug("makking it here??");
		     
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
			datapkt->size = sizeof(*valBlob);
		     }
		     
		     datapkt->timestamp_sec = 0;
		     datapkt->timestamp_nsec = 0;
		     
		    //WE SEND IN TWO PACKETS: REPLY AND DATA PCKET
		     //for the third argument void* data: send in NULL for the reply and value Blob for the data
		     //we want to send in the VALUE from GET in the packet, which is stored in newVal

		     //  void* datax = NULL; 
		     proto_send_packet(connfd, reppkt, NULL); //send after making REPLY packet
		     
		     //  if(proto_send_packet(connfd, datapkt, valBlob->content) == -1)
		     proto_send_packet(connfd, datapkt, newVal); //send after making REPLY packet

		    // debug("DAMMM I MADE IT FAR");	

		     if(gstat == TRANS_ABORTED){ //if abort or commit, break but if pending, thats good!
			trans_abort(trans); //effects of an aborted trans are removed from the 
			break;
		     }

		     debug("ITS OVERRRR");
		     // key_dispose(tempKey);
		     // blob_unref(newVal);
		     
		   
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

		//this is the end of the while loop
		// Free(reqpkt);
		// Free(datapkt);
		// Free(reppkt);
	}

	creg_unregister(client_registry, connfd);
	trans_unref(trans, "Terminating client service thread");
	
	Close(connfd);
	return NULL;
}
