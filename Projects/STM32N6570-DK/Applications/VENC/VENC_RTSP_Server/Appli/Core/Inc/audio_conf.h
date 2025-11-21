/**
******************************************************************************
* @file    audio_conf.h
* @author  MCD Application Team
* @brief   Audio configuration header
******************************************************************************
* @attention
*
* Copyright (c) 2018-2022 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component. If no LICENSE file is
* provided, the software is provided AS-IS.
*
******************************************************************************
*/

#ifndef _AUDIO_CONF_H_
#define _AUDIO_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported macros -----------------------------------------------------------*/
#define SECTION_DMA ".noncacheable"

#if defined(__ICCARM__)
/* IAR: place variable in section and align to 4 bytes */
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section) \
  _Pragma("data_alignment=4") type var @ section
#elif defined(__GNUC__)
/* GNU: use section attribute and alignment */
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section) \
  __attribute__((__section__(section))) type var __attribute__ ((aligned(4)))
#elif defined(__CC_ARM)
/* ARMCC: use section attribute and align directive */
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section) \
  __attribute__((__section__(section))) __align(4) type var
#else
#error "Unsupported compiler for VAR_DECLARE_ALIGN4_AT_SECTION"
#endif

#ifdef __cplusplus
}
#endif
#endif /* _AUDIO_CONF_H_ */

  
  
  


