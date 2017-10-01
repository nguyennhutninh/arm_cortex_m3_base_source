#ifndef __TASK_GW_IF_H__
#define __TASK_GW_IF_H__

#include "../common/cmd_line.h"

#include "../rf_protocols/RF24/RF24.h"
#include "../rf_protocols/RF24Network/RF24Network.h"
#include "../rf_protocols/RF24Network/RF24Network_config.h"

#include "app_data.h"

#define RF24_BUFFER_SIZE		128

extern RF24 nrf24l01;
extern RF24Network local_network;
extern network_config_t ac_network_config;

#endif // __TASK_GW_IF_H__
