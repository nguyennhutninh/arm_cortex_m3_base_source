/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "../ak/ak.h"
#include "app_if.h"
#include "app_eeprom.h"
#include "app_data.h"

/*****************************************************************************/
/* FIRMWARE task define
 */
/*****************************************************************************/
/* define timer */
#define FW_PACKED_TIMEOUT_INTERVAL			(5000)
#define FW_CHECKING_INTERVAL				(500)

/* define signal */
enum {
	FW_CRENT_APP_FW_INFO_REQ = AK_USER_DEFINE_SIG,
	FW_CRENT_BOOT_FW_INFO_REQ,
	FW_UPDATE_REQ,
	FW_UPDATE_SM_OK,
	FW_TRANSFER_REQ,
	FW_INTERNAL_UPDATE_APP_RES_OK,
	FW_INTERNAL_UPDATE_BOOT_RES_OK,
	FW_SAFE_MODE_RES_OK,
	FW_UPDATE_SM_BUSY,
	FW_PACKED_TIMEOUT,
	FW_CHECKING_REQ
};

/*****************************************************************************/
/*  LIFE task define
 */
/*****************************************************************************/
/* define timer */
#define AC_LIFE_TASK_TIMER_LED_LIFE_INTERVAL		(1000)

/* define signal */
enum {
	AC_LIFE_SYSTEM_CHECK = AK_USER_DEFINE_SIG,
};

/*****************************************************************************/
/*  SHELL task define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
enum {
	AC_SHELL_LOGIN_CMD = AK_USER_DEFINE_SIG,
	AC_SHELL_REMOTE_CMD,
};

/*****************************************************************************/
/*  RF24 task define
 */
/*****************************************************************************/
/* private define */
#define AC_RF24_IF_WATCH_DOG_COUNTER_MAX			30		/* 30' */

/* define timer */
#define AC_RF24_IF_POLLING_INTERVAL					(100)	/* 100 ms */
#define AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL		(100)	/* 100 ms */

/* define signal */
enum {
	AC_RF24_IF_IRQ_TX_FAIL = AK_USER_DEFINE_SIG,
	AC_RF24_IF_IRQ_TX_SUCCESS,
	AC_RF24_IF_IRQ_RX_READY,
	AC_RF24_IF_INIT_NETWORK,
	AC_RF24_IF_PURE_MSG_OUT,
	AC_RF24_IF_COMMON_MSG_OUT,
	AC_RF24_IF_TIMER_PACKET_DELAY,
};

/*****************************************************************************/
/* IF task define
 */
/*****************************************************************************/
/* define timer */
#define AC_IF_TIMER_PACKET_TIMEOUT_INTERVAL			(500)

/* define signal */
enum {
	AC_IF_PURE_MSG_IN = AK_USER_DEFINE_SIG,
	AC_IF_PURE_MSG_OUT,
	AC_IF_COMMON_MSG_IN,
	AC_IF_COMMON_MSG_OUT,
	AC_IF_DYNAMIC_MSG_IN,
	AC_IF_DYNAMIC_MSG_OUT,
};

/*****************************************************************************/
/* UART_IF task define
 */
/*****************************************************************************/
/* timer signal */
/* define signal */
enum {
	AC_UART_IF_PURE_MSG_OUT = AK_USER_DEFINE_SIG,
	AC_UART_IF_COMMON_MSG_OUT,
	AC_UART_IF_DYNAMIC_MSG_OUT,
};

/*****************************************************************************/
/*  LIFE task define
 */
/*****************************************************************************/
/* define timer */
#define AC_DISPLAY_LOGO_INTERVAL			(10000)

/* define signal */
enum {
	AC_DISPLAY_INITIAL = AK_USER_DEFINE_SIG,
	AC_DISPLAY_SHOW_ON_LOGO,
	AC_DISPLAY_SHOW_OFF_LOGO,
};

/*****************************************************************************/
/* DBG task define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	AC_DBG_TEST_1 = AK_USER_DEFINE_SIG,
};

/*****************************************************************************/
/*  global define variable
 */
/*****************************************************************************/
#define APP_OK									(0x00)
#define APP_NG									(0x01)

#define APP_FLAG_OFF							(0x00)
#define APP_FLAG_ON								(0x01)

/*****************************************************************************/
/*  app function declare
 */
/*****************************************************************************/
#define APP_VER		"0.0.1"

extern const char* app_version;

extern void* app_get_boot_share_data();
extern int  main_app();

#ifdef __cplusplus
}
#endif

#endif //APP_H
