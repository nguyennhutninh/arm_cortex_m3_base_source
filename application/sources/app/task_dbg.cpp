#include "fsm.h"
#include "port.h"
#include "message.h"

#include "sys_ctrl.h"
#include "sys_io.h"
#include "sys_dbg.h"

#include "utils.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_dbg.h"

#include "link_sig.h"
#include "link.h"

void task_dbg(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DBG_TEST_1: {
		APP_DBG_SIG("AC_DBG_TEST_1\n");
		uint8_t test_buf[64];
		for (int i = 0; i < 64; i++) {
			test_buf[i] = 0xAA;
		}

		ak_msg_t* s_msg = get_common_msg();
		set_if_des_type(s_msg, IF_TYPE_RF24_GW);
		set_if_src_type(s_msg, IF_TYPE_RF24_AC);
		set_if_des_task_id(s_msg, GW_TASK_DEBUG_MSG_ID);
		set_if_sig(s_msg, GW_DEBUG_MSG_2);
		set_if_data_common_msg(s_msg, test_buf, 64);

		set_msg_sig(s_msg, AC_LINK_SEND_COMMON_MSG);
		task_post(AC_LINK_ID, s_msg);
	}
		break;

	case AC_DBG_TEST_2: {
		APP_DBG_SIG("AC_DBG_TEST_2\n");
		uint8_t test_buf[64];
		for (int i = 0; i < 64; i++) {
			test_buf[i] = 0xAA;
		}

		ak_msg_t* s_msg = get_common_msg();
		set_if_des_type(s_msg, IF_TYPE_RF24_GW);
		set_if_src_type(s_msg, IF_TYPE_RF24_AC);
		set_if_des_task_id(s_msg, GW_TASK_DEBUG_MSG_ID);
		set_if_sig(s_msg, GW_DEBUG_MSG_6);
		set_if_data_common_msg(s_msg, test_buf, 64);

		set_msg_sig(s_msg, AC_LINK_SEND_COMMON_MSG);
		task_post(AC_LINK_ID, s_msg);
	}
		break;

	case AC_DBG_TEST_3: {
		APP_DBG_SIG("AC_DBG_TEST_3\n");
	}
		break;

	default:
		break;
	}
}
