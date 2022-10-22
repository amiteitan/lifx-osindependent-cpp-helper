#ifndef _COMM_WRAPPER_WIN_UDP_H_
#define _COMM_WRAPPER_WIN_UDP_H_
#include<iostream>

#include<winsock2.h>
#include<ws2tcpip.h>

#include "comm_wrapper.h"
class CommWrapperWinUDP : public CommWrapper
{
private:
  WSADATA wsaData;
  SOCKET sock;
  fd_set read_set;
  timeval time_to_wait;
  
 public:
	CommWrapperWinUDP(bool broadcast = false)
	{ 
		bool ret_val = true;
        std::string error_message = "";
		int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsa_result != 0) {
			error_message = "Error in setting Broadcast option";
			std::cout << error_message;
            ret_val = false;
		}

		sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (broadcast) 
		{
			char broadcast = '1';
			if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,	sizeof(broadcast)) < 0) 
			{
				error_message = "Error in setting Broadcast option";
                std::cout << error_message << std::endl;
                
				closesocket(sock);
                ret_val = false;
			}  
		}
        
		//This will be a noneblocking socket with timeout of 1 second
		int timeout = 1000;
		time_to_wait.tv_sec = static_cast<long>(timeout / 1000);
        time_to_wait.tv_usec = static_cast<long>((timeout % 1000) * 1000);
        FD_ZERO(&read_set);
        // add the listening socket to read_set
        FD_SET(sock, &read_set);
	}
    
	virtual int Send(CommUDPMessage& message)
	{
        struct sockaddr_in Recv_addr;
        Recv_addr.sin_family = AF_INET;
        Recv_addr.sin_port = htons(message.port);
        Recv_addr.sin_addr.s_addr =
            inet_addr(message.ip_address.c_str());            
		return sendto(sock, message.message, message.length, 0,
                        (sockaddr*)&Recv_addr, sizeof(Recv_addr));
	}

    virtual int Receive(CommUDPMessage& message)
	{
		FD_ZERO(&read_set);
		// add the listening socket to read_set
		FD_SET(sock, &read_set);
		int status = select(0, &read_set, 0, 0, &time_to_wait);
		int received_bytes = 0;
		if (status != 0)
		{
			struct sockaddr_in Sender_addr;
			char str[INET_ADDRSTRLEN];
			int len = sizeof(struct sockaddr_in);
			received_bytes = recvfrom(sock, message.message, message.length, 0, (sockaddr*)&Sender_addr, &len);
			inet_ntop(AF_INET, &(Sender_addr.sin_addr), str, INET_ADDRSTRLEN);
			message.ip_address = str;
			message.length = received_bytes;
		}
		else
		{
			std::cout << "no waiting messages" << std::endl;
		}

		
        return received_bytes;
	}
};
#endif //_COMM_WRAPPER_WIN_UDP_H_