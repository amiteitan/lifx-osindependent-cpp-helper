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

#define BUFLEN 512	//Max length of buffer
#define MYPORT 56700    // the port users will be connecting to
using namespace std;


class CLightInfo
{
public:
	char id[9];
	int last_ip_byte;
	CLightInfo()
	{
		memset(id, 0, 8);
	}
	
	~CLightInfo()
	{
		memset(id, 0, 8);
	}
};



int run_2()
{
	WSADATA wsaData;
	
	std::map<uint32_t, CLightInfo*> lights_container;

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

	Recv_addr.sin_addr.s_addr = inet_addr("10.175.245.255");  // switch doesnt work
	 
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
				decode_lifx_message(recvbuff, recvbufflen);
				get_lifx_device_id(recvbuff, recvbufflen, new_light_info->id, 8);
				new_light_info->last_ip_byte = ip_last_byte;
				lights_container[ip_address] = new_light_info;

			}
		}
		
		
	} while (try_receive);
	printf("=======================================");
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
		//D073D52192BB00
		if (last_ip_byte == 160)
		{
			light_parameters light_params;
			/*
			light_params.brightness = 1;
			light_params.hue = 336;
			light_params.saturation = 0;
			light_params.power = true;
			light_params.kelvin = 3500;
			*/
			light_params.brightness = 1;
			light_params.hue = 300;
			light_params.saturation = 0;
			light_params.power = false;
			light_params.kelvin = 3500;
			//len2 = build_set_color_message(sendMSG, BUFLEN, light_params);
			len2 = build_set_power_message(sendMSG, BUFLEN, true);
			sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
			Sleep(1000);
		}
		len2 = build_get_version_message(sendMSG, 100);
		sendto(sock, sendMSG, len2, 0, (sockaddr*)&Recv_addr, sizeof(Recv_addr));
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
				decode_lifx_message(recvbuff, BUFLEN);
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

int main()
{
	run_2();
	//run_1();
 }