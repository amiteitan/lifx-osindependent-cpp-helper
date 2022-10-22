using namespace std;
#include "lifxlan.h"

/*
	Simple udp client
*/
#include<stdio.h>
#include<winsock2.h>
#include<stdlib.h>
#include <iostream>
//#include <vector>
#include <map>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include "þþcomm_wrapper_win_udp.h"
#include "þþcomm_message.h"

#include "lifx_light.h"

#define BUFLEN 512	//Max length of buffer
#define MYPORT 56700    // the port users will be connecting to
using namespace std;


class CLightInfo2
{
public:
	char id[9];
	int last_ip_byte;
	string ip_address;
	CLightInfo2()
	{
		memset(id, 0, 8);
	}

	bool equals(char* other_id)
	{
		bool retVal = true;
		for (int i = 0; i < 7; i++)
		{
			if (id[i] != other_id[i])
			{
				retVal = false;
				break;
			}
		}
		return retVal;
	}
	
	~CLightInfo2()
	{
		memset(id, 0, 8);
	}
};

bool discovery()
{
	
	return false;
}


int run_2()
{
	WSADATA wsaData;
	
	std::map<uint32_t, CLightInfo*> lights_container;
	light_info decoded_light_info;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	char broadcast = '1';
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		cout << "Error in setting Broadcast option";
		closesocket(sock);
		return 0;
	}

	
	
	struct sockaddr_in Recv_addr;
	struct sockaddr_in Sender_addr;
	int len = sizeof(struct sockaddr_in);
	char sendMSG[BUFLEN];
	int len2 = build_discovery_message(sendMSG, 100);
	char recvbuff[BUFLEN] = "";
	int recvbufflen = BUFLEN;
	Recv_addr.sin_family = AF_INET;
	Recv_addr.sin_port = htons(MYPORT);

	Recv_addr.sin_addr.s_addr = inet_addr("192.168.0.255");  // switch doesnt work
	 
	// setup a timeval structure for use in select later in the program
	int timeout = 1000;
	timeval time_to_wait;
	time_to_wait.tv_sec = static_cast<long>(timeout / 1000);
	time_to_wait.tv_usec = static_cast<long>((timeout % 1000) * 1000);
	fd_set read_set;
	// initialize read_set
	FD_ZERO(&read_set);
	// add the listening socket to read_set
	FD_SET(sock, &read_set);
	///
	
	
	sendto(sock, sendMSG,len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
	Sleep(1000);
	bool try_receive = true;
	do
	{
		FD_ZERO(&read_set);
		// add the listening socket to read_set
		FD_SET(sock, &read_set);
		int status = select(0, &read_set, 0, 0, &time_to_wait);
		if (status == 0)
		{
			try_receive = false;
		}
		else
		{
			recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr*)&Sender_addr, &len);
			auto ip_address = Sender_addr.sin_addr.S_un.S_addr;
			auto ip_last_byte = Sender_addr.sin_addr.S_un.S_un_b.s_b4;
			auto val = lights_container.find(ip_address);
			if (val == lights_container.end())
			{
				CLightInfo* new_light_info = new CLightInfo();
				printf("^^^^^^^^^^^^^^^^^^^^^^\n");
				printf("message from ip: %d\n", ip_last_byte);
				decode_lifx_message(recvbuff, recvbufflen, new_light_info->decoded_light_info);
				get_lifx_device_id(recvbuff, recvbufflen, new_light_info->id, 8);
				new_light_info->ip_address = ip_address;
				lights_container[ip_address] = new_light_info;

			}
		}
	} while (try_receive);
	  ("=======================================");
	for (auto& ip : lights_container) {
		
		Recv_addr.sin_addr.S_un.S_addr = ip.first;
		int last_ip_byte = Recv_addr.sin_addr.S_un.S_un_b.s_b4;
 		cout << "****************" << endl;
		cout << "Commands are sent to this IP: " << (int)last_ip_byte << endl;
		/*
		len2 = build_discovery_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		len2 = build_get_power_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		*/
		char wanted_id[] = { 0xD0, 0x73, 0xD5, 0x21, 0x92, 0xBB, 0x00 };
		if (ip.second->equals(wanted_id))
		{
			cout << "Found the light!!" << endl;;
			light_parameters light_params;
			/*
			light_params.brightness = 1;
			light_params.hue = 336;
			light_params.saturation = 0;
			light_params.power = true;
			light_params.kelvin = 3500;
			*/
			light_params.brightness = 1;
 			light_params.saturation = 0;
			light_params.power = false;
			light_params.kelvin = 3500;
			//len2 = build_set_color_message(sendMSG, BUFLEN, light_params);
			len2 = build_set_power_message(sendMSG, BUFLEN, false);
			sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
			Sleep(1000);
		}
		/*
		len2 = build_get_version_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		*/
		len2 = build_get_color_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		Sleep(1000);
		/*
		len2 = build_get_color_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		len2 = build_get_color_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
		*/
		cout << "\nWaiting for replys...\n";
		try_receive = true;
		do
		{
			FD_ZERO(&read_set);
			// add the listening socket to read_set
			FD_SET(sock, &read_set);

			int status = select(0, &read_set, 0, 0, &time_to_wait);
			if (status == -1)
			{
				perror("Select fail Reson: ");
				printf("Error in socket %d\n", WSAGetLastError());
			}
			if (status == 0)
			{
				try_receive = false;  
							
			}
			else
			{
				recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr*)&Sender_addr, &len);
				decode_lifx_message(recvbuff, BUFLEN, &decoded_light_info);
			}
		} while (try_receive);

		
	
		
	}

	

	closesocket(sock);
	WSACleanup();
	
	//Free allocated memory
	for (auto& ip : lights_container) {

		delete ip.second;
	}
	lights_container.clear();
}

void run()
{ 
	light_info decoded_light_info;
	std::map<string, CLightInfo*> lights_container;
	CommWrapper* comm_channel = new CommWrapperWinUDP(true);
	CommUDPMessage outgoing_message;
	CommUDPMessage incomming_message;
	char sendMSG[BUFLEN];
	incomming_message.message = sendMSG;
	incomming_message.length = BUFLEN;
	//Send discovery message
	string network_broadcast_address = "192.168.0.255";
	outgoing_message.ip_address = network_broadcast_address;
	outgoing_message.port = MYPORT;
	outgoing_message.message = sendMSG;
	outgoing_message.length = build_discovery_message(sendMSG, BUFLEN);
	comm_channel->Send(outgoing_message);
	Sleep(1000);
	while (comm_channel->Receive(incomming_message) > 0)
	{	
			auto val = lights_container.find(incomming_message.ip_address);
			if (val == lights_container.end())
			{
				CLightInfo* new_light_info = new CLightInfo();
				std::cout << "received message from: " << incomming_message.ip_address << std::endl;
				decode_lifx_message(incomming_message.message, incomming_message.length ,&decoded_light_info);
				get_lifx_device_id(incomming_message.message, incomming_message.length, new_light_info->id, 8);
				new_light_info->ip_address = incomming_message.ip_address;
				lights_container[incomming_message.ip_address] = new_light_info;
			}
			incomming_message.length = BUFLEN; //Set the maxLen for the next cycle
	}
	
	
	for (auto& ip : lights_container)
	{

	}
	
	//Release memory below this comment
	//Free allocated memory
	for (auto& ip : lights_container)
	{

		delete ip.second;
	}
	lights_container.clear();

	delete comm_channel;
}

void run_optimal()
{
	CommWrapper* comm_channel = new CommWrapperWinUDP(true);
	char wanted_id[] = { 0xD0, 0x73, 0xD5, 0x21, 0x92, 0xBB, 0x00 };
	LifxLightDeviceContainer my_device(comm_channel);
	string network_broadcast_address = "192.168.0.255";
	if (my_device.init(network_broadcast_address))
	{
		auto light_mika_room = my_device.find_light(wanted_id);
		auto light_mika_desk = my_device.find_light("MikaDesk");

		my_device.turn_on(*light_mika_room);
		for (int i = 0; i < 10; i++)
		{
			my_device.turn_off(*light_mika_room);
			my_device.turn_off(*light_mika_desk);
			Sleep(6000);
			my_device.turn_on(*light_mika_room);
			my_device.turn_on(*light_mika_desk);
			Sleep(6000);
		}
	}
	delete comm_channel;
	
}

int main()
{
	run_optimal();
	//run();
	//run_2();
	
  }