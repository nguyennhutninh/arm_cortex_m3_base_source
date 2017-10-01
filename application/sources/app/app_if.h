/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   30/1/2017
 * @brief:  app external signal define
 ******************************************************************************
**/
#ifndef __APP_IF_H__
#define __APP_IF_H__
#include <stdint.h>

/*****************************************************************************/
/* task GW_SYS define.
 */
/*****************************************************************************/
/* define timer */
/* define signal */
#define GW_SYS_WATCH_DOG_REPORT_REQ					(1)

/*****************************************************************************/
/*  task GW_RF24 define.
 */
/*****************************************************************************/
/* define timer */
#define GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL		(100)

/* define signal */
#define GW_RF24_IF_PURE_MSG_OUT						(1)
#define GW_RF24_IF_COMMON_MSG_OUT					(2)
#define GW_RF24_IF_TIMER_PACKET_DELAY				(3)

/*****************************************************************************/
/*  task GW_CONSOLE define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
#define GW_CONSOLE_INTERNAL_LOGIN_CMD				(1)

/*****************************************************************************/
/* task GW_IF define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
#define GW_IF_PURE_MSG_IN							(1)
#define GW_IF_PURE_MSG_OUT							(2)
#define GW_IF_COMMON_MSG_IN							(3)
#define GW_IF_COMMON_MSG_OUT						(4)
#define GW_IF_DYNAMIC_MSG_IN						(5)
#define GW_IF_DYNAMIC_MSG_OUT						(6)

/*****************************************************************************/
/*  getway task firmware define
 */
/*****************************************************************************/
/* define timer */
#define GW_TIMER_FIRMWARE_TRANFER_PACKET_PENDING_INTERVAL	(100)		/* 100ms */
#define GW_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL			(5000)		/* 5s */
#define GW_TIMER_FIRMWARE_GET_FIRMWARE_INFO_TIMEOUT			(2000)		/* 2s */
#define GW_TIMER_FIRMWARE_DEV_INTERNAL_UPDATE_TIMEOUT		(180000)	/* 3' */

/* define signal */
#define GW_FW_OTA_REQ								(1)
#define GW_FW_CURRENT_INFO_RES						(2)
#define GW_FW_UPDATE_RES_OK							(3)
#define GW_FW_TRANFER_REQ							(4)
#define GW_FW_TRANSFER_RES_OK						(5)
#define GW_FW_TRANSFER_CHECKSUM_ERR					(6)
#define GW_FW_INTERNAL_UPDATE_REQ					(7)
#define GW_FW_UPDATE_BUSY							(8)
#define GW_FW_PACKED_TIMEOUT						(9)
#define GW_FW_UPDATE_COMPLETED						(10)
#define GW_FW_SM_UPDATE_RES_OK						(11)
#define GW_FW_SM_RELEASE_RES_OK						(12)
#define GW_FW_GET_FIRMWARE_INFO_TIMEOUT				(13)
#define GW_FW_DEV_INTERNAL_UPDATE_TIMEOUT			(14)

#endif //__APP_IF_H__
