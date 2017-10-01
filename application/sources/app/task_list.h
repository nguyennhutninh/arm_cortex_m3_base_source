#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "../ak/ak.h"
#include "../ak/task.h"

extern task_t app_task_table[];

/*****************************************************************************/
/*  DECLARE: Internal Task ID
 *  Note: Task id MUST be increasing order.
 */
/*****************************************************************************/
/**
  * SYSTEM TASKS
  **************/
#define TASK_TIMER_TICK_ID				0

/**
  * APP TASKS
  **************/
#define AC_TASK_FW_ID					1
#define AC_TASK_SHELL_ID				2
#define AC_TASK_LIFE_ID					3
#define AC_TASK_IF_ID					4
#define AC_TASK_RF24_IF_ID				5
#define AC_TASK_UART_IF_ID				6
#define AC_TASK_DBG_ID					7
#define AC_TASK_DISPLAY_ID				8

/**
  * EOT task ID
  **************/
#define AK_TASK_EOT_ID					9

/*****************************************************************************/
/*  DECLARE: Task entry point
 */
/*****************************************************************************/
extern void task_fw(ak_msg_t*);
extern void task_shell(ak_msg_t*);
extern void task_life(ak_msg_t*);
extern void task_if(ak_msg_t*);
extern void task_rf24_if(ak_msg_t*);
extern void task_uart_if(ak_msg_t*);
extern void task_dbg(ak_msg_t*);
extern void task_display(ak_msg_t*);

#endif //__TASK_LIST_H__
