/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

/* kernel include */
#include "../ak/ak.h"
#include "../ak/message.h"
#include "../ak/timer.h"
#include "../ak/fsm.h"

/* driver include */
#include "../driver/led/led.h"
#include "../driver/button/button.h"
#include "../driver/flash/flash.h"
#include "../driver/hs1101/hs1101.h"

/* app include */
#include "app.h"
#include "app_dbg.h"
#include "app_bsp.h"
#include "app_flash.h"
#include "app_non_clear_ram.h"

#include "task_list.h"
#include "task_shell.h"
#include "task_life.h"
#include "task_if.h"
#include "task_rf24_if.h"

/* sys include */
#include "../sys/sys_boot.h"
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"
#include "../sys/sys_ctrl.h"
#include "../sys/sys_dbg.h"
#include "../sys/sys_arduino.h"

/* common include */
#include "../common/utils.h"
#include "../common/screen_manager.h"

const char* app_version = APP_VER;

static boot_app_share_data_t boot_app_share_data;

static void app_power_on_reset();
static void app_start_timer();
static void app_init_state_machine();
static void app_task_init();

/*****************************************************************************/
/* app main function.
 */
/*****************************************************************************/
int main_app() {
	APP_PRINT("app version: %s\n", app_version);

	sys_soft_reboot_counter++;

	/******************************************************************************
	* init active kernel
	*******************************************************************************/
	ENTRY_CRITICAL();
	task_init();
	task_create(app_task_table);
	EXIT_CRITICAL();

	/******************************************************************************
	* init applications
	*******************************************************************************/
	/*********************
	* hardware configure *
	**********************/
	/* init watch dog timer */
	sys_ctrl_independent_watchdog_init();	/* 32s */
	sys_ctrl_soft_watchdog_init(200);		/* 20s */

	SPI.begin();

	/* adc peripheral configure */
	io_cfg_adc1();			/* configure adc for thermistor and CT sensor */

	/* adc configure for ct sensor */
	adc_ct_io_cfg();

	/* flash io init */
	flash_io_ctrl_init();

	/*********************
	* software configure *
	**********************/
	/* initial boot object */
	sys_boot_init();

	/* life led init */
	led_init(&led_life, led_life_init, led_life_on, led_life_off);

	/* button init */
	button_init(&btn_mode,	10,	BUTTON_MODE_ID,	io_button_mode_init,	io_button_mode_read,	btn_mode_callback);
	button_init(&btn_up,	10,	BUTTON_UP_ID,	io_button_up_init,		io_button_up_read,		btn_up_callback);
	button_init(&btn_down,	10,	BUTTON_DOWN_ID,	io_button_down_init,	io_button_down_read,	btn_down_callback);

	button_enable(&btn_mode);
	button_enable(&btn_up);
	button_enable(&btn_down);

	/* get boot share data */
	flash_read(APP_FLASH_INTTERNAL_SHARE_DATA_SECTOR_1, (uint8_t*)&boot_app_share_data, sizeof(boot_app_share_data_t));
	if (boot_app_share_data.is_power_on_reset == SYS_POWER_ON_RESET) {
		app_power_on_reset();
	}

	/* increase start time */
	fatal_log_t app_fatal_log;
	flash_read(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR, (uint8_t*)&app_fatal_log, sizeof(fatal_log_t));
	app_fatal_log.restart_times ++;
	flash_erase_sector(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR);
	flash_write(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR, (uint8_t*)&app_fatal_log, sizeof(fatal_log_t));

	/* get server settings info */
	eeprom_read(EEPROM_APP_NETWORK_CONFIG_ADDR, (uint8_t*)&ac_network_config, sizeof(network_config_t));

	EXIT_CRITICAL();

	/* start timer for application */
	app_init_state_machine();
	app_start_timer();

	/******************************************************************************
	* app task initial
	*******************************************************************************/
	app_task_init();

	/******************************************************************************
	* run applications
	*******************************************************************************/
	return task_run();
}

/*****************************************************************************/
/* app initial function.
 */
/*****************************************************************************/

/* start software timer for application
 * used for app tasks
 */
void app_start_timer() {
	/* start timer to toggle life led */
	timer_set(AC_TASK_LIFE_ID, AC_LIFE_SYSTEM_CHECK, AC_LIFE_TASK_TIMER_LED_LIFE_INTERVAL, TIMER_PERIODIC);

	/* checking firmware update */
	timer_set(AC_TASK_FW_ID, FW_CHECKING_REQ, FW_CHECKING_INTERVAL, TIMER_ONE_SHOT);
}

/* init state machine for tasks
 * used for app tasks
 */
void app_init_state_machine() {
}

/* send first message to trigger start tasks
 * used for app tasks
 */
void app_task_init() {
	{
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_RF24_IF_INIT_NETWORK);
		task_post(AC_TASK_RF24_IF_ID, s_msg);
	}

	{
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_DISPLAY_INITIAL);
		task_post(AC_TASK_DISPLAY_ID, s_msg);
	}
}

/*****************************************************************************/
/* app common function
 */
/*****************************************************************************/

/* hardware timer interrupt 10ms
 * used for led, button polling
 */
void sys_irq_timer_10ms() {
	button_timer_polling(&btn_mode);
	button_timer_polling(&btn_up);
	button_timer_polling(&btn_down);
}

/* hardware timer interrupt 50ms
 * used for encode and decode ir
 */
void sys_irq_timer_50us() {
}

/* hardware rtc interrupt alarm
 * used for internal rtc alarm
 */
void sys_irq_timer_hs1101() {
}

/* hardware io interrupt at rev ir pin
 * used for decode ir
 */
void sys_irq_ir_io_rev() {
}

/* init non-clear RAM objects
 */
void app_power_on_reset() {
	sys_soft_reboot_counter = 0;
}

void* app_get_boot_share_data() {
	return (void*)&boot_app_share_data;
}
