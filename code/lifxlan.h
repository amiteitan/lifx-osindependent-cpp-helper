#ifndef _LIFX_LAN_HELPER_H_
#define _LIFX_LAN_HELPER_H_
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <math.h>
using namespace std;
//Based on documentations by lifx https://lan.developer.lifx.com/docs/encoding-a-packet
#pragma pack(push, 1)
#define MESSAGE_SOURCE 0x42424242
#define HAVE_PRINTF 1
typedef struct {
  /* frame */
  uint16_t size;
  uint16_t protocol:12;
  uint16_t  addressable:1;
  uint16_t  tagged:1;
  uint16_t  origin:2;
  uint32_t source;
  /* frame address */
  uint8_t  target[8];
  uint8_t  reserved[6];
  uint8_t  res_required:1;
  uint8_t  ack_required:1;
  uint8_t  :6;
  uint8_t  sequence;
  /* protocol header */
  uint64_t :64;
  uint16_t type;
  uint16_t :16;
  /* variable length payload follows */
} lx_protocol_header_t;

typedef struct {
    lx_protocol_header_t header;
    uint8_t  reserved;
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint16_t duration;
} tx_lx_setcolor_Packet102;

typedef struct {
    lx_protocol_header_t header;
    uint16_t power;
}tx_lx_setpower_Packet21;

//RX
typedef struct {
  lx_protocol_header_t header;
  uint8_t service; //1 is UDP; others are reserved and are not for us
  uint32_t portOfService; //Uusally 56700 but don't assume this is allways the case
}rx_stateService_Packet003;

typedef struct {
    lx_protocol_header_t header;
    uint16_t power;
}rx_statePower_Packet22;



typedef struct {
  lx_protocol_header_t header;
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
  uint8_t reserved6[2];
  uint16_t power;
  uint8_t label[32];
  uint8_t reserved7[8];
}rx_lightState_Packet107;

typedef struct {
    lx_protocol_header_t header;
    uint32_t vendor;
    uint32_t product;
    char reserved[4];
}rx_stateVersion_Packet33;



typedef struct {
    uint16_t hue;
    float saturation;
    float brightness;
    uint16_t kelvin;
    bool power;
}light_parameters;

typedef struct
{
    uint8_t id[8];
    uint8_t label[32];
    uint32_t vendor;
    uint32_t product;
    light_parameters params;
}light_info;

int build_tx_message_base(char* raw_message, uint16_t size, uint16_t type = 2)
{
    lx_protocol_header_t* message = (lx_protocol_header_t*)raw_message;
    memset(message, 0, sizeof(lx_protocol_header_t));
    message->type = 2;
    message->protocol = 1024;
    message->size = sizeof(lx_protocol_header_t);
    message->source = MESSAGE_SOURCE;
    message->addressable = 1;
    message->tagged = 1;
    message->res_required = true;
    message->ack_required = false;
    message->type = type;
    return message->size;
}

//GET MESSAGE
int build_discovery_message(char* raw_message, uint16_t size)
{
    int len = build_tx_message_base(raw_message, size,2);
    return len;
}

int build_get_power_message(char* raw_message, uint16_t size)
{
    int len = build_tx_message_base(raw_message, size,20);
    return len;
}

int build_get_color_message(char* raw_message, uint16_t size)
{
    int len = build_tx_message_base(raw_message, size,101);
    return len;
}

int build_get_version_message(char* raw_message, uint16_t size)
{
    int len = build_tx_message_base(raw_message, size,32);
    return len;
}
//SET MESSAGE
int build_set_color_message(char* raw_message, uint16_t size, light_parameters light_params)
{
    int len = sizeof(tx_lx_setcolor_Packet102);
    build_tx_message_base(raw_message, size, 102);
    tx_lx_setcolor_Packet102* message = (tx_lx_setcolor_Packet102*)raw_message;
    uint16_t hue = int(round(0x10000 * light_params.hue) / 360) % 0x10000;
    uint16_t saturation = int(round(0xFFFF * light_params.saturation));
    uint16_t brightness = int(round(0xFFFF * light_params.brightness));
    message->duration = 1;
    message->hue = hue;
    message->brightness = brightness;
    message->saturation = saturation;
    message->kelvin = light_params.kelvin;
    message->header.size = len;
    return len;
}

int build_set_power_message(char* raw_message, uint16_t size, bool power)
{
    
    int len = sizeof(tx_lx_setpower_Packet21);
    build_tx_message_base(raw_message, size, 21);
    tx_lx_setpower_Packet21* message = (tx_lx_setpower_Packet21*)raw_message;
    message->power = power ? 0xFFFF : 0;
    return len;
}





int decode_lifx_header(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
    lx_protocol_header_t* message = (lx_protocol_header_t*)raw_message;
    memset(decoded_light_info->id, 0, 8);
    for (int i = 0; i < 8; i++)
    {
        decoded_light_info->id[i] = message->target[i];
    }

#ifdef HAVE_PRINTF
    printf("-------------------\n");
    printf("Target: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%X", decoded_light_info->id[i]);
    }
    printf("\n");
    printf("Message type = %d\n", message->type);
    printf("Message Len = %d\n", message->size);
#endif
    return message->type;
}

bool get_lifx_device_id(char* raw_message, uint16_t size, char* id_buf, uint16_t id_buf_len)
{
    memset(id_buf, 0, id_buf_len);
    bool ret_value = false;
    if (id_buf_len == 8)
    {
        lx_protocol_header_t* message = (lx_protocol_header_t*)raw_message;
        for (int i = 0; i < 8; i++)
        {
            id_buf[i] = message->target[i];
        }
        ret_value = true;
    }
    return ret_value;
}

void decode_lifx_statePower(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
    rx_statePower_Packet22* message = (rx_statePower_Packet22*)raw_message;
#ifdef HAVE_PRINTF
    printf("power = %x\n", message->power);
#endif
} 


void decode_lifx_stateService(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
    rx_stateService_Packet003* message = (rx_stateService_Packet003*)raw_message;
#ifdef HAVE_PRINTF
    printf("service = %d\n", message->service);
    printf("port = %d\n", message->portOfService);
#endif
}

void decode_lifx_lightState(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
    rx_lightState_Packet107* message = (rx_lightState_Packet107*)raw_message;
    for (int i = 0; i < 32; i++)
    {
        decoded_light_info->label[i] = message->label[i];
    }
    light_parameters* light_params = &(decoded_light_info->params);
    light_params->hue = round((double(message->hue) * 360 / 0x10000) * 100) / 100; //Round to 2 decimal places
    light_params->brightness = round((double(message->brightness) / 0xFFFF) * 10000) / 10000; //Round to 4 decimal places
    light_params->saturation = round((double(message->saturation) / 0xFFFF) * 10000) / 10000; //Round to 4 decimal places
    light_params->power = message->power != 0;
    light_params->kelvin = message->kelvin;

#ifdef HAVE_PRINTF
    for (int i = 0; i < 32; i++)
    {
        printf("%c", message->label[i]);
    }
    printf("\n");
    printf("hue = %d\n", light_params->hue);
    printf("saturation = %.4f\n", light_params->saturation);
    printf("brightness = %.4f\n", light_params->brightness);
    printf("kelvin = %d\n", light_params->kelvin);
    printf("power = %s\n", light_params->power ? "TRUE" : "FALSE");
#endif
}

void decode_lifx_stateVersion(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
    rx_stateVersion_Packet33* message = (rx_stateVersion_Packet33*)raw_message;
    decoded_light_info->vendor = message->vendor;
    decoded_light_info->product = message->product;

#ifdef HAVE_PRINTF
    printf("vendor: %d\n", message->vendor);
    printf("id: %d ", message->product);
    switch (message->product)
    {
    case 27:
        printf("LIFX A19");
        break;
    case 60:
        printf("LIFX Mini White to Warm");
        break;
    case 61:
        printf("LIFX MINI White");
        break;
    default:
        printf("Uncategorized");
        break;
    }
    printf("\n");
#endif

}

void  decode_lifx_message(char* raw_message, uint16_t size, light_info* decoded_light_info)
{
  int type = decode_lifx_header(raw_message, size, decoded_light_info);
  void (*decode_func)(char*, uint16_t, light_info*) = nullptr;
  //StateService - packet3
  switch (type)
  {
  case 3:
      decode_func = decode_lifx_stateService;
      break;
  case 22:
      decode_func = decode_lifx_statePower;
      break;
  case 107:
      decode_func = decode_lifx_lightState;
      break;
  case 33:
      decode_func = decode_lifx_stateVersion;
  }

  if (decode_func != nullptr)
  {
      decode_func(raw_message, size, decoded_light_info);
  }
  
   //light
   //Light State packet_107
}
#pragma pack(pop)

#endif // _LIFX_LAN_HELPER_H_