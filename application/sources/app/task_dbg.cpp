#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_ctrl.h"

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

	default:
		break;
	}
}
