#ifndef _LIFX_LIGHT_H_
#define _LIFX_LIGHT_H_
#include <iostream>
#include <map>

#include "lifxlan.h"
#include "comm_wrapper.h"

class CLightInfo
{
private:
	light_info m_decoded_light_info;
public:
	light_info* decoded_light_info;
	string ip_address;
	char id[9];
	
	
	CLightInfo()
	{
		memset(id, 0, 8);
		decoded_light_info = &m_decoded_light_info;
	}

	bool equals(const char* other_id)
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

	bool equals(string other_id)
	{
		return this->equals(other_id.c_str());
	}

	~CLightInfo()
	{
		memset(id, 0, 8);
	}
};

class LifxLightDeviceContainer
{
private:
	//std::string device_id;
	//std::string device_label;
	

	std::map<string, CLightInfo*> lights_container;
	CommWrapper* comm_channel;
	CommUDPMessage outgoing_message;
	CommUDPMessage incomming_message;
	
	static const int BUFLEN = 512;	//Max length of buffer
	static const int DEFAULT_LIFX_PORT = 56700;	// the port users will be connecting to	
	char messages_buffer[BUFLEN];

	bool set_power_state(CLightInfo& light,bool power_state)
	{
		bool retval = false;
		if (&light != nullptr)
		{
			outgoing_message.ip_address = light.ip_address;
			outgoing_message.port = DEFAULT_LIFX_PORT;
			outgoing_message.length = build_set_power_message(outgoing_message.message, BUFLEN, power_state);
			int bytes_sent = comm_channel->Send(outgoing_message);
			if (bytes_sent > 0)
			{
				retval = true;
			}
			while (comm_channel->Receive(incomming_message) > 0)
			{
				decode_lifx_message(incomming_message.message, incomming_message.length, light.decoded_light_info);
				incomming_message.length = BUFLEN; //Set the maxLen for the next cycle
			}

		}
		return retval;
	}
public:


	LifxLightDeviceContainer(CommWrapper* comm_channel)
	{
		this->comm_channel = comm_channel;
		memset(messages_buffer, 0, BUFLEN);
		outgoing_message.message = messages_buffer;
		incomming_message.message = messages_buffer;
	}

	~LifxLightDeviceContainer()
	{
		for (auto& light : lights_container)
		{
			delete(light.second);
		}
	}

	
	bool init(string outgoing_address)
	{
		return this->refresh(outgoing_address);
	}

	bool refresh(string outgoing_address)
	{
		outgoing_message.ip_address = outgoing_address;
		outgoing_message.port = DEFAULT_LIFX_PORT;
		outgoing_message.length = build_discovery_message(outgoing_message.message, BUFLEN);
		int bytes_sent = comm_channel->Send(outgoing_message);
		Sleep(1000);
		incomming_message.length = BUFLEN; 
		while (comm_channel->Receive(incomming_message) > 0)
		{
			auto val = lights_container.find(incomming_message.ip_address);
			if (val == lights_container.end())
			{
				CLightInfo* new_light_info = new CLightInfo();
				std::cout << "received message from: " << incomming_message.ip_address << std::endl;
				decode_lifx_message(incomming_message.message, incomming_message.length, new_light_info->decoded_light_info);
				get_lifx_device_id(incomming_message.message, incomming_message.length, new_light_info->id, 8);
				new_light_info->ip_address = incomming_message.ip_address;
				lights_container[incomming_message.ip_address] = new_light_info;
			}
			incomming_message.length = BUFLEN; //Set the maxLen for the next cycle
		}
		

 		for (auto& light : lights_container)
		{
			outgoing_message.ip_address = light.first;
			outgoing_message.port = DEFAULT_LIFX_PORT;
			outgoing_message.length = build_get_color_message(outgoing_message.message, BUFLEN);
			int bytes_sent = comm_channel->Send(outgoing_message);
			incomming_message.length = BUFLEN; //Set the maxLen for the next cycle
			while (comm_channel->Receive(incomming_message) > 0)
			{
				decode_lifx_message(incomming_message.message, incomming_message.length, light.second->decoded_light_info);
				incomming_message.length = BUFLEN; //Set the maxLen for the next cycle
			}

		}
		return lights_container.size() > 0;

	}

	

	CLightInfo* find_light(char* id)
	{
		CLightInfo* found_the_light = nullptr;
		for (auto& light : lights_container)
		{  
			bool retVal = true;
			for (int i = 0; i < 7; i++)
			{
				if (id[i] != light.second->id[i])
				{
					retVal = false;
					break;
				}
			}
			if (retVal)
			{
				std::cout << "found_light" << std::endl;
				found_the_light = light.second;
			}
		}
		return found_the_light;
	}

	CLightInfo* find_light(string label)
	{
		CLightInfo* found_the_light = nullptr;
		
 		for (auto& light : lights_container)
		{
			if (light.second->decoded_light_info->label.compare(0, label.size(), label) != 0)
			{
				continue;
			}
			std::cout << "found_light" << std::endl;
			found_the_light = light.second;
			break;
		}
		
		return found_the_light;
	}

	bool turn_on(CLightInfo& light)
	{
		std::cout << "turn_on" << std::endl;
		return set_power_state(light, true);
	}

	bool turn_off(CLightInfo& light)
	{
		std::cout << "turn_off" << std::endl;
		return set_power_state(light, false);
	}

	bool toggle(CLightInfo& light)
	{
		std::cout << "toggle" << std::endl;
		bool current_power = light.decoded_light_info->params.power;
		return set_power_state(light, !current_power);
	}

	bool set_parameters(light_parameters color)
	{
		std::cout << "set_parameters" << std::endl;
	}


};

#endif // _LIFX_LIGHT_H_