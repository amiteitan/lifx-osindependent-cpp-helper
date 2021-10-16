#ifndef _COMM_WRAPPER_H_
#define _COMM_WRAPPER_H_
#include "þþcomm_message.h"
class CommWrapper
{
public:
  virtual int Send(CommUDPMessage &message) = 0;
 virtual int Receive(CommUDPMessage &message) = 0;
};
#endif //_COMM_WRAPPER_H_