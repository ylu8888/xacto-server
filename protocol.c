int proto_send_packet(int fd, XACTO_HEADER *pkt, void *data){
  //first convert the multi-byte fields in packet from normal host byte order to network byte order
  pkt->serial = htonl(pkt->serial);
  pkt->size = htonl(pkt->size);
  pkt->timestamp_sec = htonl(pkt->timestamp_sec);
  pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);
   
  //then write the header to the network connection
  int writeRez = write(fd, pkt, sizeof(XACTO_PACKET));
  if(writeRez < 0) return -1; //error handler

  //if length of header is not zero, call write again to write payload data to network conn

  if(data != NULL && pkt->length != 0){
    int writeRes = write(fd, data, nhtol(pkt->length)); //nhtol is network to host long

    if(writeRes < 0) return -1;
  }
  
  return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
  
}
