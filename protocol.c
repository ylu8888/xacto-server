#include "protocol.h"
#include "csapp.h"

int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
  //first convert the multi-byte fields in packet from normal host byte order to network byte order
  pkt->serial = htonl(pkt->serial);
  pkt->size = htonl(pkt->size);
  pkt->timestamp_sec = htonl(pkt->timestamp_sec);
  pkt->timestamp_nsec = htonl(pkt->timestamp_nsec); //htonl is host long to network
   
  //then write the packet header to the network connection
  int writeRez = write(fd, pkt, sizeof(pkt));
  if(writeRez < 0) return -1; //error handler

  //if length of header is not zero, call write again to write payload data to network connection
  if(data != NULL && pkt->size != 0){
    int writeRes = write(fd, data, htonl(pkt->size)); //nhtol is network to host long

    if(writeRes < 0) return -1;
  }
  
  return 0;
}

// int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
  
// }
