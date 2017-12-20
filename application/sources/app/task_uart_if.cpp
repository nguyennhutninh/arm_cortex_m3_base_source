#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../common/cmd_line.h"
#include "../common/utils.h"
#include "../common/xprintf.h"

#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"
#include "../sys/sys_ctrl.h"
#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_uart_if.h"
#include "task_list.h"

#define RF24_SIG_DBG_EN		0

#define IFCPU_SOP_CHAR					(0xEF)
#define IFCPU_DATA_SIZE					(254)

#define SOP_STATE						(0x00)
#define LEN_STATE						(0x01)
#define DATA_STATE						(0x02)
#define FCS_STATE						(0x03)

struct if_uart_frame_t {
	uint8_t		frame_sop;
	uint32_t	len;
	uint8_t		data_index;
	uint8_t		data[IFCPU_DATA_SIZE];
	uint8_t		frame_fcs;
} if_uart_frame;

static uint8_t uart_rx_frame_state = SOP_STATE;
static uint8_t tx_buffer[IFCPU_DATA_SIZE];

uint8_t rx_frame_parser(uint8_t data);
static void tx_frame_post(uint8_t*, uint32_t);
static void dync_msg_frame_post(ak_msg_dynamic_if_t*);
static uint8_t if_uart_calcfcs(uint8_t, uint8_t*);

void task_uart_if(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UART_IF_PURE_MSG_OUT: {
#if (RF24_SIG_DBG_EN == 1)
		APP_DBG("[IF UART][SEND] AC_UART_IF_PURE_MSG_OUT\n");
#endif
		ak_msg_pure_if_t app_if_msg;

		/* assign if message */
		app_if_msg.header.type			= PURE_MSG_TYPE;
		app_if_msg.header.if_src_type	= msg->if_src_type;
		app_if_msg.header.if_des_type	= msg->if_des_type;
		app_if_msg.header.sig			= msg->if_sig;
		app_if_msg.header.src_task_id	= msg->if_src_task_id;
		app_if_msg.header.des_task_id	= msg->if_des_task_id;

		tx_frame_post((uint8_t*)&app_if_msg, sizeof(ak_msg_pure_if_t));
	}
		break;

	case AC_UART_IF_COMMON_MSG_OUT: {
#if (RF24_SIG_DBG_EN == 1)
		APP_DBG("[IF UART][SEND] AC_UART_IF_COMMON_MSG_OUT\n");
#endif
		ak_msg_common_if_t app_if_msg;

		/* assign if message */
		app_if_msg.header.type			= COMMON_MSG_TYPE;
		app_if_msg.header.if_src_type	= msg->if_src_type;
		app_if_msg.header.if_des_type	= msg->if_des_type;
		app_if_msg.header.sig			= msg->if_sig;
		app_if_msg.header.src_task_id	= msg->if_src_task_id;
		app_if_msg.header.des_task_id	= msg->if_des_task_id;

		app_if_msg.len = get_data_len_common_msg(msg);
		memcpy(app_if_msg.data, get_data_common_msg(msg), sizeof(uint8_t) * app_if_msg.len);

		tx_frame_post((uint8_t*)&app_if_msg, sizeof(ak_msg_common_if_t));
	}
		break;

	case AC_UART_IF_DYNAMIC_MSG_OUT: {
#if (RF24_SIG_DBG_EN == 1)
		APP_DBG("[IF UART][SEND] AC_UART_IF_DYNAMIC_MSG_OUT\n");
#endif
		ak_msg_dynamic_if_t app_if_msg;

		/* assign if message */
		app_if_msg.header.type			= DYNAMIC_MSG_TYPE;
		app_if_msg.header.if_src_type	= msg->if_src_type;
		app_if_msg.header.if_des_type	= msg->if_des_type;
		app_if_msg.header.sig			= msg->if_sig;
		app_if_msg.header.src_task_id	= msg->if_src_task_id;
		app_if_msg.header.des_task_id	= msg->if_des_task_id;

		app_if_msg.len = get_data_len_dynamic_msg(msg);

		app_if_msg.data = (uint8_t*)ak_malloc(app_if_msg.len);

		get_data_dynamic_msg(msg, (uint8_t*)app_if_msg.data, app_if_msg.len);
		dync_msg_frame_post(&app_if_msg);

		ak_free(app_if_msg.data);
	}
		break;

	case AC_UART_IF_FRAME_TO: {
		uart_rx_frame_state = SOP_STATE;
	}
		break;

	default:
		break;
	}
}

uint8_t rx_frame_parser(uint8_t ch) {
	uint8_t is_parser_frame = APP_FLAG_ON;

	switch (uart_rx_frame_state) {
	case SOP_STATE: {
		if (IFCPU_SOP_CHAR == ch) {
			uart_rx_frame_state = LEN_STATE;
			timer_set(AC_TASK_UART_IF_ID, AC_UART_IF_FRAME_TO, AC_UART_IF_FRAME_TO_INTERVAL, TIMER_ONE_SHOT);
		}
		else {
			is_parser_frame = APP_FLAG_OFF;
		}
	}
		break;

	case LEN_STATE: {
		if (ch > IFCPU_DATA_SIZE) {
			timer_remove_attr(AC_TASK_UART_IF_ID, AC_UART_IF_FRAME_TO);

			uart_rx_frame_state = SOP_STATE;
			is_parser_frame = APP_FLAG_OFF;
		}
		else {
			if_uart_frame.len = ch;
			if_uart_frame.data_index = 0;
			uart_rx_frame_state = DATA_STATE;

			timer_set(AC_TASK_UART_IF_ID, AC_UART_IF_FRAME_TO, (AC_UART_IF_FRAME_TO_INTERVAL * if_uart_frame.len), TIMER_ONE_SHOT);
		}
	}
		break;

	case DATA_STATE: {
		if_uart_frame.data[if_uart_frame.data_index++] = ch;

		if (if_uart_frame.data_index == if_uart_frame.len) {
			uart_rx_frame_state = FCS_STATE;
		}
	}
		break;

	case FCS_STATE: {
		timer_remove_attr(AC_TASK_UART_IF_ID, AC_UART_IF_FRAME_TO);

		uart_rx_frame_state = SOP_STATE;

		if_uart_frame.frame_fcs = ch;

		if (if_uart_frame.frame_fcs \
				== if_uart_calcfcs(if_uart_frame.len, if_uart_frame.data)) {

			ak_msg_if_header_t* if_msg_header = (ak_msg_if_header_t*)if_uart_frame.data;

			switch (if_msg_header->type) {
			case PURE_MSG_TYPE: {
#if (RF24_SIG_DBG_EN == 1)
				APP_DBG("[IF UART][REV] PURE_MSG_TYPE\n");
#endif
				ak_msg_t* s_msg = get_pure_msg();

				set_if_src_task_id(s_msg, if_msg_header->src_task_id);
				set_if_des_task_id(s_msg, if_msg_header->des_task_id);
				set_if_src_type(s_msg, if_msg_header->if_src_type);
				set_if_des_type(s_msg, if_msg_header->if_des_type);
				set_if_sig(s_msg, if_msg_header->sig);

				set_msg_sig(s_msg, AC_IF_PURE_MSG_IN);
				task_post(AC_TASK_IF_ID, s_msg);
			}
				break;

			case COMMON_MSG_TYPE: {
#if (RF24_SIG_DBG_EN == 1)
				APP_DBG("[IF UART][REV] COMMON_MSG_TYPE\n");
#endif
				ak_msg_t* s_msg = get_common_msg();

				set_if_src_task_id(s_msg, if_msg_header->src_task_id);
				set_if_des_task_id(s_msg, if_msg_header->des_task_id);
				set_if_src_type(s_msg, if_msg_header->if_src_type);
				set_if_des_type(s_msg, if_msg_header->if_des_type);
				set_if_sig(s_msg, if_msg_header->sig);
				set_if_data_common_msg(s_msg, ((ak_msg_common_if_t*)if_msg_header)->data, ((ak_msg_common_if_t*)if_msg_header)->len);

				set_msg_sig(s_msg, AC_IF_COMMON_MSG_IN);
				task_post(AC_TASK_IF_ID, s_msg);
			}
				break;

			case DYNAMIC_MSG_TYPE: {
#if (RF24_SIG_DBG_EN == 1)
				APP_DBG("[IF UART][REV] DYNAMIC_MSG_TYPE\n");
#endif
				ak_msg_t* s_msg = get_dynamic_msg();

				set_if_src_task_id(s_msg, if_msg_header->src_task_id);
				set_if_des_task_id(s_msg, if_msg_header->des_task_id);
				set_if_src_type(s_msg, if_msg_header->if_src_type);
				set_if_des_type(s_msg, if_msg_header->if_des_type);
				set_if_sig(s_msg, if_msg_header->sig);

				uint8_t* data_msg = ((uint8_t*)if_msg_header + sizeof(ak_msg_if_header_t) + sizeof(uint32_t));
				set_if_data_dynamic_msg(s_msg, data_msg, ((ak_msg_dynamic_if_t*)if_msg_header)->len);

				set_msg_sig(s_msg, AC_IF_DYNAMIC_MSG_IN);
				task_post(AC_TASK_IF_ID, s_msg);
			}
				break;

			default:
				FATAL("IF_UART", 0x02);
				break;
			}
		}
		else {
			/* TODO: handle checksum incorrect */
			FATAL("IF_UART", 0x03);
		}
	}
		break;

	default:
		break;
	}

	return is_parser_frame;
}

void tx_frame_post(uint8_t* data, uint32_t len) {
	tx_buffer[0] = IFCPU_SOP_CHAR;
	tx_buffer[1] = len;
	memcpy(&tx_buffer[2], data, len);
	tx_buffer[2 + len] = if_uart_calcfcs(len, data);

	for (uint8_t i = 0; i < len + 3; i++) {
		sys_ctrl_shell_put_char(tx_buffer[i]);
	}
}

void dync_msg_frame_post(ak_msg_dynamic_if_t* ak_msg_dynamic) {
	uint32_t dynamic_message_len = sizeof(ak_msg_if_header_t) + sizeof(uint32_t) + ak_msg_dynamic->len;
	uint32_t dynamic_message_header_len = sizeof(ak_msg_if_header_t) + sizeof(uint32_t);

	if (dynamic_message_len > 255) {
		FATAL("IF_UART", 0x04);
	}

	/* calculate checksum */
	uint8_t xor_result = dynamic_message_len;
	uint8_t* raw_data = (uint8_t*)ak_msg_dynamic;
	/* checksum header + len */
	for (uint32_t i = 0; i < dynamic_message_header_len; i++, raw_data++) {
		xor_result = xor_result ^ *raw_data;
	}
	/* checksum data */
	for (uint32_t i = 0; i < ak_msg_dynamic->len; i++) {
		xor_result = xor_result ^ ak_msg_dynamic->data[i];
	}

	tx_buffer[0] = IFCPU_SOP_CHAR; /* start of frame */
	tx_buffer[1] = dynamic_message_len; /* len of frame */
	memcpy(&tx_buffer[2], ak_msg_dynamic, dynamic_message_header_len); /* copy header to tx frame */
	memcpy(&tx_buffer[2 + dynamic_message_header_len], ak_msg_dynamic->data, ak_msg_dynamic->len); /* copy message data */
	tx_buffer[2 + dynamic_message_len] = xor_result; /* checksum */

	/* put frame through physic layer */
	for (uint8_t i = 0; i < dynamic_message_len + 3; i++) {
		sys_ctrl_shell_put_char(tx_buffer[i]);
	}
}

uint8_t if_uart_calcfcs(uint8_t len, uint8_t *data_ptr) {
	uint8_t xor_result = len;
	for (int i = 0; i < len; i++, data_ptr++) {
		xor_result = xor_result ^ *data_ptr;
	}

	return xor_result;
}
