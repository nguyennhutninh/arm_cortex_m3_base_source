#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_ctrl.h"
#include "../sys/sys_io.h"
#include "../sys/sys_dbg.h"

#include "../common/utils.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_dbg.h"

void task_dbg(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DBG_TEST_1: {
		APP_DBG_SIG("AC_DBG_TEST_1\n");
	}
		break;

	case AC_DBG_TEST_2: {
		APP_DBG_SIG("AC_DBG_TEST_2\n");
	}
		break;

	case AC_DBG_TEST_3: {
		APP_DBG_SIG("AC_DBG_TEST_3\n");
		uint8_t* rev_data = (uint8_t*)ak_malloc(150);
		get_data_dynamic_msg(msg, rev_data, 150);
		for (uint8_t i = 0; i < 150; i++) {
			if (*(rev_data + i) != i) {
				FATAL("DYNC", 0x01);
			}
		}
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_ON_LOGO);
	}
		break;

	default:
		break;
	}
}
