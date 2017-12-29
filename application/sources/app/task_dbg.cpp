#include "fsm.h"
#include "port.h"
#include "message.h"

#include "sys_ctrl.h"
#include "sys_io.h"
#include "sys_dbg.h"

#include "utils.h"

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
