#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_ctrl.h"
#include "../sys/sys_io.h"

#include "../common/utils.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_dbg.h"

void task_dbg(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DBG_TEST_1: {
		APP_DBG("AC_DBG_TEST_1\n");
		uint8_t data[64];
		memcpy(data, get_data_common_msg(msg), 64);
		for (int i = 0; i < 64; i++) {
			xprintf("%d ", data[i]);
		}
		xprintf("\n");

		ak_msg_t* s_msg = get_pure_msg();
		set_if_des_type(s_msg, IF_TYPE_RF24_GW);
		set_if_src_type(s_msg, IF_TYPE_RF24_AC);
		set_if_des_task_id(s_msg, AC_TASK_DBG_ID);
		set_if_sig(s_msg, AC_DBG_TEST_2);

		set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_DBG_TEST_2: {
		APP_DBG("AC_DBG_TEST_2\n");
		uint8_t data[64];
		memcpy(data, get_data_common_msg(msg), 64);
		for (int i = 0; i < 64; i++) {
			xprintf("%d ", data[i]);
		}
		xprintf("\n");

		ak_msg_t* s_msg = get_pure_msg();
		set_if_des_type(s_msg, IF_TYPE_RF24_GW);
		set_if_src_type(s_msg, IF_TYPE_RF24_AC);
		set_if_des_task_id(s_msg, AC_TASK_DBG_ID);
		set_if_sig(s_msg, AC_DBG_TEST_2);

		set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	default:
		break;
	}
}
