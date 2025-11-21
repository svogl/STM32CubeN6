/**
 ******************************************************************************
 * @file    Appli/Core/Inc/venc_h264_config.h
 * @author  MCD Application Team
 * @brief   VENC support public header
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may obtain a copy of the License in the LICENSE file
 * in the root directory of this software component.
 ******************************************************************************
 */

#ifndef __PLUGIN_VENC_H__
#define __PLUGIN_VENC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdint.h>
#include "stm32n6xx_hal.h"
#include "stm32n6xx_ll_venc.h"
#include "H264EncApi.h"
#include "venc_h264_config.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief  Camera configuration for H.264 (dimensions).
 */
typedef struct cam_h264_cfg_t
{
  uint32_t width;  /**< frame width in pixels */
  uint32_t height; /**< frame height in pixels */
} cam_h264_cfg_t;

/**
 * @brief  DCMIPP (pixel packer) configuration.
 */
typedef struct dcmipp_h264_cfg_t
{
  uint32_t format; /**< pixel packer format identifier */
  uint32_t pitch;  /**< line pitch in bytes */
} dcmipp_h264_cfg_t;

/**
 * @brief  Aggregated VENC H.264 configuration (encoder API structures).
 */
typedef struct venc_h264_cfg_t
{
  H264EncConfig           cfgH264Main;     /**< main H.264 config */
  H264EncPreProcessingCfg cfgH264Preproc;  /**< pre-processing config */
  H264EncCodingCtrl       cfgH264Coding;   /**< coding control config */
  H264EncRateCtrl         cfgH264Rate;     /**< rate control config */
} venc_h264_cfg_t;

/* Exported variables --------------------------------------------------------*/
extern cam_h264_cfg_t     hCamH264Instance;
extern dcmipp_h264_cfg_t  hDcmippH264Instance;
extern venc_h264_cfg_t    hVencH264Instance;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Provide encoder working buffer pool choice to EWL.
 * @param  pool_ptr Pointer to buffer pointer (output)
 * @param  size     Pointer to size (output)
 */
void EWLPoolChoiceCb(u8 **pool_ptr, size_t *size);

/**
 * @brief  Get a pointer to the input frame buffer and its size.
 * @param  framSize Output: frame size in bytes.
 * @retval uint8_t* Pointer to input frame data.
 */
uint8_t *GetInputFrame(uint32_t *framSize);

/**
 * @brief  Return whether hardware handshake (slice/stream) mode is active.
 * @retval bool true if hw handshake mode enabled.
 */
bool IsHwHanshakeMode(void);

/**
 * @brief  Get DCMIPP pixel packer format id used by the pipeline.
 * @retval uint32_t format identifier
 */
uint32_t GetDCMIPPFormat(void);

/**
 * @brief  Get picture type expected for the encoder input.
 * @retval H264EncPictureType picture type
 */
H264EncPictureType GetInputFrameType(void);

/**
 * @brief  Retrieve DCMIPP lines configuration (warp/irq lines).
 * @param  pWarpLines Output warp lines count
 * @param  pIrqLines  Output irq lines count
 * @retval HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef GetDCMIPPLinesConfig(uint32_t *pWarpLines, uint32_t *pIrqLines);

/**
 * @brief  Return number of captured lines by DCMIPP.
 * @retval uint32_t number of lines captured
 */
uint32_t GetDCMIPPNbLinesCaptured(void);

#ifdef __cplusplus
}
#endif

#endif /* __PLUGIN_VENC_H__ */


