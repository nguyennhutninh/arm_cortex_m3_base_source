/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef __PORT_H__
#define __PORT_H__

#if defined(TIVA_PLATFORM)
#include "tiva.h"
#elif defined(STM32L_PLATFORM)
#include "stm32l.h"
#elif defined(STM32F10X_PLATFORM)
#include "stm32.h"
#else
#error Please choose platform for kernel.
#endif

#define __AK_PACKETED	__attribute__((__packed__))
#define __AK_WEAK		__attribute__((__weak__))

#define __AK_MALLOC_CTRL_SIZE	8

#endif //__PORT_H__
