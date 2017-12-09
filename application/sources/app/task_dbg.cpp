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
	}
		break;

	default:
		break;
	}
}
