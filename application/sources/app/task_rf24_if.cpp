#include <stdbool.h>
#include <stdint.h>

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_rf24_if.h"
#include "task_if.h"
#include "task_list.h"

#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"

#include "../common/utils.h"
#include "../common/fifo.h"

#include "../driver/eeprom/eeprom.h"
#include "../driver/exor/exor.h"

#define SEND_FIFO_BUFFER_SIZE			10

RF24 nrf24l01(1, 2);
RF24Network local_network(nrf24l01);
network_config_t ac_network_config;

static uint8_t rf24_buffer[RF24_BUFFER_SIZE];

static uint8_t rf24_encypt_decrypt_buffer[RF24_BUFFER_SIZE];

static ak_msg_common_if_t send_common_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_common_fifo;

static ak_msg_pure_if_t send_pure_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_pure_fifo;

static uint8_t rf_pending_flag = APP_FLAG_OFF;

void sys_irq_nrf24l01() {
#if defined(RF24_USED_POLLING)

#else
	bool tx_ok, tx_fail, rx_ready;
	nrf24l01.whatHappened(tx_ok, tx_fail, rx_ready);

	if (tx_ok) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_TX_SUCCESS);
		task_post(AC_TASK_RF24_IF_ID, msg);
	}

	if (tx_fail) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_TX_FAIL);
		task_post(AC_TASK_RF24_IF_ID, msg);
	}

	if (rx_ready) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_RX_READY);
		task_post(AC_TASK_RF24_IF_ID, msg);
	}
#endif
}

void task_rf24_if(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_RF24_IF_IRQ_RX_READY: {
		/* rf24network polling */
		local_network.update();

		if (local_network.available()) {
			memset(rf24_buffer, 0, RF24_BUFFER_SIZE);
			RF24NetworkHeader rf24_rev_header;
			local_network.read(rf24_rev_header, rf24_buffer, RF24_BUFFER_SIZE);

			switch (rf24_rev_header.type) {

			case RF24_DATA_COMMON_MSG_TYPE: {
				APP_DBG_SIG("RF24_DATA_COMMON_MSG_TYPE\n");

				rf24_encrypt_decrypt_key[0] = (uint8_t)(rf24_rev_header.id % 0xFF);
				rf24_encrypt_decrypt_key[1] = (uint8_t)((rf24_rev_header.id >> 8) % 0xFF);
				rf24_encrypt_decrypt_key[2] = rf24_rev_header.type;
				exor_encrypt_decrypt(rf24_buffer, rf24_encypt_decrypt_buffer, sizeof(ak_msg_common_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

				ak_msg_common_if_t* if_msg = (ak_msg_common_if_t*)rf24_encypt_decrypt_buffer;

				if (if_msg->header.if_des_type == IF_TYPE_RF24_AC) {
					ak_msg_t* s_msg = get_common_msg();

					set_if_src_task_id(s_msg, if_msg->header.src_task_id);
					set_if_des_task_id(s_msg, if_msg->header.des_task_id);
					set_if_src_type(s_msg, if_msg->header.if_src_type);
					set_if_des_type(s_msg, if_msg->header.if_des_type);
					set_if_sig(s_msg, if_msg->header.sig);
					set_if_data_common_msg(s_msg, if_msg->data, if_msg->len);

					set_msg_sig(s_msg, AC_IF_COMMON_MSG_IN);
					task_post(AC_TASK_IF_ID, s_msg);

					timer_set(AC_TASK_RF24_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
				}

				rf_pending_flag = APP_FLAG_ON;
			}
				break;

			case RF24_DATA_PURE_MSG_TYPE: {
				APP_DBG_SIG("RF24_DATA_PURE_MSG_TYPE\n");

				rf24_encrypt_decrypt_key[0] = (uint8_t)(rf24_rev_header.id % 0xFF);
				rf24_encrypt_decrypt_key[1] = (uint8_t)((rf24_rev_header.id >> 8) % 0xFF);
				rf24_encrypt_decrypt_key[2] = rf24_rev_header.type;
				exor_encrypt_decrypt(rf24_buffer, rf24_encypt_decrypt_buffer, sizeof(ak_msg_pure_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

				ak_msg_pure_if_t* if_msg = (ak_msg_pure_if_t*)rf24_encypt_decrypt_buffer;

				if (if_msg->header.if_des_type == IF_TYPE_RF24_AC) {
					ak_msg_t* s_msg = get_pure_msg();

					set_if_src_task_id(s_msg, if_msg->header.src_task_id);
					set_if_des_task_id(s_msg, if_msg->header.des_task_id);
					set_if_src_type(s_msg, if_msg->header.if_src_type);
					set_if_des_type(s_msg, if_msg->header.if_des_type);
					set_if_sig(s_msg, if_msg->header.sig);

					set_msg_sig(s_msg, AC_IF_PURE_MSG_IN);
					task_post(AC_TASK_IF_ID, s_msg);

					timer_set(AC_TASK_RF24_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
				}

				rf_pending_flag = APP_FLAG_ON;
			}
				break;

			default:
				break;
			}
		}
	}
		break;

	case AC_RF24_IF_INIT_NETWORK: {
		APP_DBG_SIG("AC_RF24_IF_INIT_NETWORK\n");

		ac_network_config.node_addr			= 0;
		ac_network_config.node_chanel		= 90;
		ac_network_config.node_server_addr	= 1;

		fifo_init(&send_common_fifo, memcpy, send_common_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_common_if_t));
		fifo_init(&send_pure_fifo, memcpy, send_pure_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_pure_if_t));

		nrf24l01.begin();

#if defined(RF24_USED_POLLING)
		nrf24l01.maskIRQ(0, 0, 0);  /* disable rx interrupt */
#else
		nrf24l01.maskIRQ(0, 0, 1);  /* enable rx interrupt */
#endif
		local_network.begin(ac_network_config.node_chanel, ac_network_config.node_addr);

#if defined(RF24_USED_POLLING)
		timer_set(AC_TASK_RF24_IF_ID, AC_RF24_IF_IRQ_RX_READY, AC_RF24_IF_POLLING_INTERVAL, TIMER_PERIODIC);
#endif

#if 0
		nrf24l01.printDetails();
#endif
	}
		break;

	case AC_RF24_IF_PURE_MSG_OUT: {
		APP_DBG_SIG("AC_RF24_IF_PURE_MSG_OUT\n");

		ak_msg_pure_if_t if_msg;
		memset(&if_msg, 0, sizeof(ak_msg_pure_if_t));

		if_msg.header.src_task_id = msg->if_src_task_id;
		if_msg.header.des_task_id = msg->if_des_task_id;
		if_msg.header.type = get_msg_type(msg);
		if_msg.header.if_src_type = IF_TYPE_RF24_AC;
		if_msg.header.if_des_type = msg->if_des_type;
		if_msg.header.sig = msg->if_sig;

		if (rf_pending_flag == APP_FLAG_OFF) {
			RF24NetworkHeader server_header(ac_network_config.node_server_addr, RF24_DATA_PURE_MSG_TYPE);

			rf24_encrypt_decrypt_key[0] = (uint8_t)(server_header.id % 0xFF);
			rf24_encrypt_decrypt_key[1] = (uint8_t)((server_header.id >> 8) % 0xFF);
			rf24_encrypt_decrypt_key[2] = server_header.type;
			exor_encrypt_decrypt((uint8_t*)&if_msg, rf24_encypt_decrypt_buffer, sizeof(ak_msg_pure_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

			local_network.write(server_header, rf24_encypt_decrypt_buffer, sizeof(ak_msg_pure_if_t));
		}
		else {
			fifo_put(&send_pure_fifo, &if_msg);
		}
	}
		break;

	case AC_RF24_IF_COMMON_MSG_OUT: {
		APP_DBG_SIG("AC_RF24_IF_COMMON_MSG_OUT\n");

		ak_msg_common_if_t if_msg;
		memset(&if_msg, 0, sizeof(ak_msg_common_if_t));

		if_msg.header.src_task_id = msg->if_src_task_id;
		if_msg.header.des_task_id = msg->if_des_task_id;
		if_msg.header.type = get_msg_type(msg);
		if_msg.header.if_src_type = IF_TYPE_RF24_AC;
		if_msg.header.if_des_type = msg->if_des_type;
		if_msg.header.sig = msg->if_sig;

		if_msg.len = get_data_len_common_msg(msg);
		memcpy(if_msg.data, get_data_common_msg(msg), if_msg.len);

		if (rf_pending_flag == APP_FLAG_OFF) {
			RF24NetworkHeader server_header(ac_network_config.node_server_addr, RF24_DATA_COMMON_MSG_TYPE);

			rf24_encrypt_decrypt_key[0] = (uint8_t)(server_header.id % 0xFF);
			rf24_encrypt_decrypt_key[1] = (uint8_t)((server_header.id >> 8) % 0xFF);
			rf24_encrypt_decrypt_key[2] = server_header.type;
			exor_encrypt_decrypt((uint8_t*)&if_msg, rf24_encypt_decrypt_buffer, sizeof(ak_msg_common_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

			local_network.write(server_header, rf24_encypt_decrypt_buffer, sizeof(ak_msg_common_if_t));
		}
		else {
			fifo_put(&send_common_fifo, &if_msg);
		}
	}
		break;

	case AC_RF24_IF_TIMER_PACKET_DELAY: {
		rf_pending_flag = APP_FLAG_OFF;
		/* TODO: check data queue */
		if (!fifo_is_empty(&send_pure_fifo)) {
			ak_msg_pure_if_t if_msg;

			fifo_get(&send_pure_fifo, &if_msg);

			RF24NetworkHeader server_header(ac_network_config.node_server_addr, RF24_DATA_PURE_MSG_TYPE);

			rf24_encrypt_decrypt_key[0] = (uint8_t)(server_header.id % 0xFF);
			rf24_encrypt_decrypt_key[1] = (uint8_t)((server_header.id >> 8) % 0xFF);
			rf24_encrypt_decrypt_key[2] = server_header.type;
			exor_encrypt_decrypt((uint8_t*)&if_msg, rf24_encypt_decrypt_buffer, sizeof(ak_msg_pure_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

			local_network.write(server_header, rf24_encypt_decrypt_buffer, sizeof(ak_msg_pure_if_t));

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_RF24_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
		else if (!fifo_is_empty(&send_common_fifo)) {
			ak_msg_common_if_t if_msg;

			fifo_get(&send_common_fifo, &if_msg);

			RF24NetworkHeader server_header(ac_network_config.node_server_addr, RF24_DATA_COMMON_MSG_TYPE);

			rf24_encrypt_decrypt_key[0] = (uint8_t)(server_header.id % 0xFF);
			rf24_encrypt_decrypt_key[1] = (uint8_t)((server_header.id >> 8) % 0xFF);
			rf24_encrypt_decrypt_key[2] = server_header.type;
			exor_encrypt_decrypt((uint8_t*)&if_msg, rf24_encypt_decrypt_buffer, sizeof(ak_msg_common_if_t), rf24_encrypt_decrypt_key, RF24_ENCRYPT_DECRYPT_KEY_SIZE);

			local_network.write(server_header, rf24_encypt_decrypt_buffer, sizeof(ak_msg_common_if_t));

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_RF24_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
	}
		break;

	default:
		break;
	}
}
