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

#include "link_sig.h"

void task_link_mac(ak_msg_t* msg) {
	switch (msg->sig) {
	case LINK_MAC_INIT: {
		APP_DBG_SIG("LINK_MAC_INIT\n");
	}
		break;

	default:
		break;
	}
}
