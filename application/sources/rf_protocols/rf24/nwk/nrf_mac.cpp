#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "../../../ak/ak.h"
#include "../../../ak/port.h"
#include "../../../ak/message.h"
#include "../../../ak/timer.h"

#include "../../../common/fifo.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "task_list.h"
#include "task_list_if.h"

#include "nrf_config.h"
#include "nrf_data.h"
#include "nrf_mac.h"
#include "nrf_phy.h"

#include "../hal/hal_nrf.h"

#define MAC_HDR_LEN				(sizeof(nrf_mac_hdr_t))
#define MAX_MAC_PAYLOAD_LEN		(MAX_PHY_PAYLOAD_LEN - MAC_HDR_LEN - sizeof(uint8_t))

#define NWK_PDU_ID_FIFO_SIZE	(NRF_NWK_MSG_POOL_SIZE * 3)

typedef struct {
	uint16_t src_mac_addr;
	uint8_t mac_seq;
	uint8_t frame_num;
	uint8_t frame_seq;
} __AK_PACKETED nrf_mac_hdr_t;

typedef struct {
	nrf_mac_hdr_t hdr;
	uint8_t payload[MAX_MAC_PAYLOAD_LEN];
	uint8_t frame_cs;
} __AK_PACKETED nrf_mac_msg_t;

static nrf_mac_msg_t nrf_mac_msg;

#define TX_MAC_FRAME_RETRY_COUNTER_MAX		3
#define TX_MAC_FRAME_RETRY_INTERVAL			20

static uint8_t tx_mac_frame_retry_counter = 0;
static uint8_t tx_mac_sequence = 0;

static uint8_t rx_mac_sequence = 0;
static nrf_nwk_pdu_t* nrf_nwk_pdu_receiving;

#define NRF_NWK_SENDING_MSG_RETRY_COUNTER_MAX	3
static uint8_t nrf_nwk_sending_msg_retry_counter = 0;
static nrf_nwk_pdu_t* nrf_nwk_pdu_sending;

enum mac_state_e {
	MAC_STATE_IDLE,
	MAC_STATE_SENDING,
};

static mac_state_e mac_state;
static mac_state_e get_mac_state();
static void set_mac_state(mac_state_e);

static uint8_t calc_frame_cs(uint8_t*, uint16_t);

static uint32_t nwk_pdu_id_buf[NWK_PDU_ID_FIFO_SIZE];
static fifo_t nwk_pdu_id_fifo;

void task_rf24_mac(ak_msg_t* msg) {
	switch (msg->sig) {
	case RF24_MAC_INIT: {
		APP_DBG_SIG("RF24_MAC_INIT\n");

		fifo_init(&nwk_pdu_id_fifo, memcpy, nwk_pdu_id_buf, NWK_PDU_ID_FIFO_SIZE, sizeof(uint32_t));

		tx_mac_sequence = 0;

		set_mac_state(MAC_STATE_IDLE);
	}
		break;

	case RF24_MAC_HANDLE_MSG_OUT: {
		APP_DBG_SIG("RF24_MAC_HANDLE_MSG_OUT\n");
		uint32_t st_nwk_pdu_id;
		memcpy(&st_nwk_pdu_id, get_data_common_msg(msg), sizeof(uint32_t));

		if (get_mac_state() == MAC_STATE_SENDING) {
			if (fifo_is_full(&nwk_pdu_id_fifo)) {
				task_post_common_msg(RF24_NWK_ID, RF24_NWK_SEND_MSG_ERR_BUSY, (uint8_t*)&st_nwk_pdu_id, sizeof(uint32_t));
			}
			else {
				fifo_put(&nwk_pdu_id_fifo, (uint8_t*)&st_nwk_pdu_id);
			}
		}
		else {
			nrf_nwk_sending_msg_retry_counter = 0;

			set_mac_state(MAC_STATE_SENDING);

			/* init tx_mac_senquence */
			tx_mac_sequence ++;

			nrf_nwk_pdu_sending = nrf_nwk_pdu_get(st_nwk_pdu_id);

			nrf_mac_msg.hdr.src_mac_addr = nrf_get_static_nwk_addr();
			nrf_mac_msg.hdr.mac_seq = tx_mac_sequence;
			nrf_mac_msg.hdr.frame_num = (nrf_nwk_pdu_sending->len / MAX_MAC_PAYLOAD_LEN) + ((nrf_nwk_pdu_sending->len % MAX_MAC_PAYLOAD_LEN) > 0);
			nrf_mac_msg.hdr.frame_seq = 0;
			APP_DBG("[MAC] nrf_mac_msg.hdr.src_mac_addr: 0x%x\n", nrf_mac_msg.hdr.src_mac_addr);
			APP_DBG("[MAC] nrf_mac_msg.hdr.mac_seq: %d\n", nrf_mac_msg.hdr.mac_seq);
			APP_DBG("[MAC] nrf_mac_msg.hdr.frame_num: %d\n", nrf_mac_msg.hdr.frame_num);
			APP_DBG("[MAC] nrf_mac_msg.hdr.frame_seq: %d\n", nrf_mac_msg.hdr.frame_seq);

			task_post_pure_msg(RF24_MAC_ID, RF24_MAC_SEND_FRAME);
		}
	}
		break;

	case RF24_MAC_SEND_FRAME: {
		APP_DBG_SIG("RF24_MAC_SEND_FRAME\n");
		if (nrf_nwk_pdu_sending->is_used) {
			if (nrf_mac_msg.hdr.frame_seq == nrf_mac_msg.hdr.frame_num) {
				/* the last frame is send */
				uint32_t nwk_pdu_id = nrf_nwk_pdu_sending->id;
				task_post_common_msg(RF24_NWK_ID, RF24_NWK_SEND_MSG_DONE, (uint8_t*)&nwk_pdu_id, sizeof(uint32_t));

				set_mac_state(MAC_STATE_IDLE);

				if (fifo_availble(&nwk_pdu_id_fifo)) {
					uint32_t get_pdu_id;
					fifo_get(&nwk_pdu_id_fifo, &get_pdu_id);
					task_post_common_msg(RF24_MAC_ID, RF24_MAC_HANDLE_MSG_OUT, (uint8_t*)&get_pdu_id, sizeof(uint32_t));
				}
			}
			else {
				memcpy(nrf_mac_msg.payload, (nrf_nwk_pdu_sending->payload + (nrf_mac_msg.hdr.frame_seq * MAX_MAC_PAYLOAD_LEN)), MAX_MAC_PAYLOAD_LEN);
				nrf_mac_msg.frame_cs = calc_frame_cs(nrf_mac_msg.payload, MAX_MAC_PAYLOAD_LEN);
				task_post_common_msg(RF24_PHY_ID, RF24_PHY_SEND_FRAME_REQ, (uint8_t*)&nrf_mac_msg, sizeof(nrf_mac_msg_t));
			}
		}
		else {
			FATAL("NRF_MAC", 0x01);
		}
	}
		break;

	case RF24_MAC_SEND_FRAME_DONE: {
		APP_DBG_SIG("RF24_MAC_SEND_FRAME_DONE\n");
		tx_mac_frame_retry_counter = 0;

		nrf_mac_msg.hdr.frame_seq++;
		task_post_pure_msg(RF24_MAC_ID, RF24_MAC_SEND_FRAME);
	}
		break;

	case RF24_MAC_SEND_FRAME_ERR: {
		APP_DBG_SIG("RF24_MAC_SEND_FRAME_ERR\n");
		if (tx_mac_frame_retry_counter++ > TX_MAC_FRAME_RETRY_COUNTER_MAX) {
			/* mac frame retry */
			timer_set(RF24_MAC_ID, RF24_MAC_SEND_FRAME, TX_MAC_FRAME_RETRY_INTERVAL, TIMER_ONE_SHOT);
		}
		else {
			if (nrf_nwk_sending_msg_retry_counter++ > NRF_NWK_SENDING_MSG_RETRY_COUNTER_MAX) {
				set_mac_state(MAC_STATE_IDLE);

				uint32_t nwk_pdu_id = nrf_nwk_pdu_sending->id;
				task_post_common_msg(RF24_NWK_ID, RF24_NWK_SEND_MSG_ERR_SDF, (uint8_t*)&nwk_pdu_id, sizeof(uint32_t));

				if (fifo_availble(&nwk_pdu_id_fifo)) {
					uint32_t get_pdu_id;
					fifo_get(&nwk_pdu_id_fifo, &get_pdu_id);
					task_post_common_msg(RF24_MAC_ID, RF24_MAC_HANDLE_MSG_OUT, (uint8_t*)&get_pdu_id, sizeof(uint32_t));
				}
			}
			else {
				/* nwk frame retry */
				tx_mac_sequence++;
				tx_mac_frame_retry_counter = 0;
				nrf_mac_msg.hdr.frame_seq = 0;
				nrf_mac_msg.hdr.mac_seq = tx_mac_sequence;
				task_post_pure_msg(RF24_MAC_ID, RF24_MAC_SEND_FRAME);
			}
		}
	}
		break;

	case RF24_MAC_RECV_FRAME: {
		APP_DBG_SIG("RF24_MAC_RECV_FRAME\n");

		timer_set(RF24_MAC_ID, RF24_MAC_RECV_FRAME_TO, RF24_MAC_REV_FRAME_TO_INTERVAL, TIMER_ONE_SHOT);

		nrf_mac_msg_t* st_nrf_mac_msg = (nrf_mac_msg_t*)get_data_common_msg(msg);
		APP_DBG("[MAC] st_nrf_mac_msg->hdr.src_mac_addr: 0x%x\n", st_nrf_mac_msg->hdr.src_mac_addr);
		APP_DBG("[MAC] st_nrf_mac_msg->hdr.mac_seq: %d\n", st_nrf_mac_msg->hdr.mac_seq);
		APP_DBG("[MAC] st_nrf_mac_msg->hdr.frame_num: %d\n", st_nrf_mac_msg->hdr.frame_num);
		APP_DBG("[MAC] st_nrf_mac_msg->hdr.frame_seq: %d\n", st_nrf_mac_msg->hdr.frame_seq);
		APP_DBG("[MAC] st_nrf_mac_msg->frame_cs: 0x%x\n", st_nrf_mac_msg->frame_cs);
		APP_DBG("[MAC] rx_mac_sequence: %d\n", rx_mac_sequence);

		uint8_t calc_fcs = calc_frame_cs(st_nrf_mac_msg->payload, MAX_MAC_PAYLOAD_LEN);

		if ( calc_fcs == st_nrf_mac_msg->frame_cs) {
			/* receiving first mac frame */
			if (st_nrf_mac_msg->hdr.mac_seq != rx_mac_sequence) {
				if(st_nrf_mac_msg->hdr.frame_seq == 0) {

					nrf_nwk_pdu_receiving = nrf_nwk_pdu_malloc();

					rx_mac_sequence = st_nrf_mac_msg->hdr.mac_seq;
					nrf_nwk_pdu_receiving->len = st_nrf_mac_msg->hdr.frame_num * MAX_MAC_PAYLOAD_LEN;
					APP_DBG("nrf_nwk_pdu_receiving->len: %d\n", nrf_nwk_pdu_receiving->len);
				}
				else {
					break;
				}
			}

			if (st_nrf_mac_msg->hdr.frame_seq <= st_nrf_mac_msg->hdr.frame_num - 1) {
				msg_inc_ref_count(msg);
				set_msg_sig(msg, RF24_MAC_HANDLE_MSG_IN);
				task_post(RF24_MAC_ID, msg);
			}
		}
		else {
			APP_DBG("[MAC] checksum err calc_frame_cs: %x\n", calc_fcs);
		}
	}
		break;

	case RF24_MAC_RECV_FRAME_TO: {
		APP_DBG_SIG("RF24_MAC_RECV_FRAME_TO\n");
		if (nrf_nwk_pdu_receiving->is_used) {
			nrf_nwk_pdu_free(nrf_nwk_pdu_receiving);
		}
	}
		break;

	case RF24_MAC_HANDLE_MSG_IN: {
		APP_DBG_SIG("RF24_MAC_HANDLE_MSG_IN\n");
		if (nrf_nwk_pdu_receiving->is_used) {
			nrf_mac_msg_t* nrf_mac_msg = (nrf_mac_msg_t*)get_data_common_msg(msg);
			memcpy((nrf_nwk_pdu_receiving->payload + (nrf_mac_msg->hdr.frame_seq * MAX_MAC_PAYLOAD_LEN)), nrf_mac_msg->payload, MAX_MAC_PAYLOAD_LEN);

			if (nrf_mac_msg->hdr.frame_seq == (nrf_mac_msg->hdr.frame_num - 1)) {
				timer_remove_attr(RF24_MAC_ID, RF24_MAC_RECV_FRAME_TO);

				uint32_t nwk_pdu_id = nrf_nwk_pdu_receiving->id;
				task_post_common_msg(RF24_NWK_ID, RF24_NWK_RECV_MSG, (uint8_t*)&nwk_pdu_id, sizeof(uint32_t));
			}
		}
	}
		break;

	default:
		break;
	}
}

mac_state_e get_mac_state() {
	return mac_state;
}

void set_mac_state(mac_state_e state) {
	mac_state = state;
	APP_DBG("[MAC] set_mac_state -> %d\n", state);
	if (mac_state != MAC_STATE_SENDING) {
		nrf_phy_switch_prx_mode();
	}
	else {
		nrf_phy_switch_ptx_mode();
	}
}

uint8_t calc_frame_cs(uint8_t* data, uint16_t len) {
	uint8_t ret = 0;
	for (uint16_t i = 0; i < len; i++) {
		ret ^= *(data + i);
	}
	return ret;
}
