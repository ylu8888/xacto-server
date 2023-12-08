#include "protocol.h"
#include "csapp.h"
#include "debug.h"

int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
  //then write the packet header to the network connection
  int writeRez = rio_writen(fd, pkt, sizeof(XACTO_PACKET));
  if(writeRez < 0) return -1; //error handler

  //if length of header is not zero, call write again to write payload data to network connection
  if(data != NULL && htonl(pkt->size) != 0){
    int writeRes = rio_writen(fd, data, htonl(pkt->size)); 

    if(writeRes < 0) return -1;
  }
  
  return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
  //debug("IM IN PROTO RECV");
  int readRez = rio_readn(fd, pkt, sizeof(XACTO_PACKET));
  if(readRez < 0) return -1;

  if(ntohl(pkt->size) != 0){
    *datap = Malloc(ntohl(pkt->size)); //only malloc if pktsize is not 0
  if(*datap == NULL) return -1;
    int readRes = rio_readn(fd, *datap, ntohl(pkt->size));
    
    if(readRes < 0){
      free(*datap);
      return -1;
    } 
  } else{
    datap = NULL;
  }

  //debug("IM FINISHED WITH PROTO RECV");

  return 0;
  
}
