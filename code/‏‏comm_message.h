#ifndef _COMM_MESSAGE_H_
#define _COMM_MESSAGE_H_
#include<iostream>

class CommUDPMessage
{
public:
  std::string ip_address;
  unsigned int port;
  
  unsigned int length;
  char* message;
};
#endif  //_COMM_MESSAGE_H_