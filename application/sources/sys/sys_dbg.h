/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   05/09/2016
 ******************************************************************************
**/
#ifndef __SYS_DBG_H__
#define __SYS_DBG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "../ak/task.h"
#include "../ak/message.h"

#if defined(TIVA_PLATFORM)
#include <stdbool.h>
#include <stdint.h>
#include "../platform/tiva/utils/uartstdio.h"

#if defined(SYS_DBG_EN)
#define SYS_DBG(fmt, ...)       UARTprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_DBG(fmt, ...)
#endif

#if defined(SYS_PRINT_EN)
#define SYS_PRINT(fmt, ...)       UARTprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_PRINT(fmt, ...)
#endif
#endif

#if defined(STM32L_PLATFORM)
#include "../common/xprintf.h"

#if defined(SYS_DBG_EN)
#define SYS_DBG(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_DBG(fmt, ...)
#endif

#if defined(SYS_PRINT_EN)
#define SYS_PRINT(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_PRINT(fmt, ...)
#endif
#endif

#if defined(STM32F10X_PLATFORM)
#include "../common/xprintf.h"

#if defined(SYS_DBG_EN)
#define SYS_DBG(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_DBG(fmt, ...)
#endif

#if defined(SYS_PRINT_EN)
#define SYS_PRINT(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define SYS_PRINT(fmt, ...)
#endif
#endif

typedef struct {
	uint32_t ipsr;
	uint32_t primask;
	uint32_t faultmask;
	uint32_t basepri;
	uint32_t control;
} m3_core_reg_t;

typedef struct {
	int8_t		string[10];
	uint8_t		code;
	task_t		current_task;
	ak_msg_t	current_active_object;
	m3_core_reg_t m3_core_reg;
	uint32_t	fatal_times;
	uint32_t	restart_times;
} fatal_log_t;

#define FATAL(s, c) \
do { \
	DISABLE_INTERRUPTS(); \
	sys_dbg_fatal((const int8_t*)s, (uint8_t)c); \
} while (0);

extern void sys_dbg_fatal(const int8_t* s, uint8_t c);

#ifdef __cplusplus
}
#endif

#endif //__SYS_DBG_H__
