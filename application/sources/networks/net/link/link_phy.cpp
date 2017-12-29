#include <stdint.h>
#include <stdlib.h>

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "ak.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "link_hal.h"
#include "link_sig.h"

#define LINK_PHY_FRAME_SIZE		32

typedef struct {
	uint8_t sof; /* start of frame */
	uint32_t des_addr; /* destination address */
	uint32_t src_addr; /* sources address */
	uint8_t type; /* type of frame */
	uint8_t sub_type; /* sub-type of frame */
	uint8_t seq_num; /* frame sequence number */
	uint8_t len; /* len of data */
	uint8_t data[LINK_PHY_FRAME_SIZE]; /* frame data buffer */
	uint8_t fcs; /* frame checksum */
} __AK_PACKETED link_phy_frame_t;

#define LINK_PHY_SEND_STATE_IDLE		0
#define LINK_PHY_SEND_STATE_SENDING		1
static uint8_t link_phy_send_state = LINK_PHY_SEND_STATE_IDLE;

#define RETRY_COUNTER_SEND_TO_MAX		3
#define RETRY_COUNTER_SEND_ERR_MAX		3
static uint8_t retry_counter_send_to;
static uint8_t retry_counter_send_err;

void task_link_phy(ak_msg_t* msg) {
	switch (msg->sig) {
	case LINK_PHY_INIT: {
		APP_DBG_SIG("LINK_PHY_INIT\n");

		/* private object init */
		link_phy_send_state = LINK_PHY_SEND_STATE_IDLE;
		retry_counter_send_to = 0;
		retry_counter_send_err = 0;
	}
		break;

	case LINK_PHY_FRAME_SEND_REQ: {
		APP_DBG_SIG("LINK_PHY_FRAME_SEND_REQ\n");
	}
		break;

	case LINK_PHY_FRAME_SEND: {
		APP_DBG_SIG("LINK_PHY_FRAME_SEND\n");
	}
		break;

	case LINK_PHY_FRAME_SEND_TO: {
		APP_DBG_SIG("LINK_PHY_FRAME_SEND_TO\n");
	}
		break;

	default:
		break;
	}
}
