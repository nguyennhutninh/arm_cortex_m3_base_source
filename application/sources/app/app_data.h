#ifndef __APP_DATA_H__
#define __APP_DATA_H__
#include <stdint.h>

#include "../sys/sys_boot.h"

#include "app.h"


/******************************************************************************
* interface type
*******************************************************************************/
/** RF24 interface for modules
 * IF_TYPE_RF24_GW using to transfer data to gateway.
 * IF_TYPE_RF24_AC using to transfer data to air_condition module.
*/
#define IF_TYPE_RF24_GW						(0)
#define IF_TYPE_RF24_AC						(1)
#define IF_TYPE_RF24_MAX					(2)

/******************************************************************************
* Data type of RF24Network
*******************************************************************************/
#define RF24_DATA_COMMON_MSG_TYPE			(1)
#define RF24_DATA_PURE_MSG_TYPE				(2)
#define RF24_DATA_REMOTE_CMD_TYPE			(3)

/** APP interface, communication via socket interface
 *
 */
#define IF_TYPE_APP_MIN						(100)
#define IF_TYPE_APP_GW						(100)
#define IF_TYPE_APP_GI						(101)

/** UART interface
 *
 */
#define IF_TYPE_UART_GW						(120)
#define IF_TYPE_UART_AC						(121)

/******************************************************************************
* Common define
*******************************************************************************/
#define IF_RETRY_COUNTER_MAX		3

/******************************************************************************
* Commom data structure for transceiver data
*******************************************************************************/
#define TOTAL_AIR_COND		4

#define AIR_COND_MODE_AUTO			1
#define AIR_COND_MODE_MANUAL		2

#define RF24_ENCRYPT_DECRYPT_KEY_SIZE		16
extern uint8_t rf24_encrypt_decrypt_key[];

typedef struct {
	uint8_t remote_temperature;
	uint8_t humindity;
	uint32_t air_cond_current[4];
} ac_sensor_packet_t;

typedef struct {
	uint16_t	time_air_counter;			/* time will increase when hr change*/
	uint8_t		time_air_range;				/* range time (hour) saved in epprom */
	uint8_t		total_air_cond;				/* total of air conditional*/
	uint8_t		total_air_cond_alternate;	/* total of air conditional alternate */
	uint8_t		mode_air_cond;				/* air-condition mode: auto/manual control */

	uint8_t		milestone_temp_cool;		/* *C */
	uint8_t		milestone_temp_normal;		/* *C */
	uint8_t		milestone_temp_hot;			/* *C */

	uint8_t		temp_calibration;			/* *C */
	uint8_t		hum_calibration;			/* %RH */

	uint8_t		operations_calib_temp;		/*  */
	uint8_t		operations_calib_hum;		/*  */
} app_setting_t;

typedef struct {
	uint8_t available;
	uint8_t failed_counter;
	uint8_t active;
	uint8_t resersed;
	uint32_t milestone_on;
} air_cond_t;

typedef struct {
	uint8_t		node_chanel;		/* node nrf chanel */
	uint8_t		node_addr;			/* node address */
	uint8_t		node_server_addr;	/* node server address*/
} network_config_t;

typedef struct {
	uint8_t is_power_on_reset;
} boot_app_share_data_t;

#endif //__APP_DATA_H__
