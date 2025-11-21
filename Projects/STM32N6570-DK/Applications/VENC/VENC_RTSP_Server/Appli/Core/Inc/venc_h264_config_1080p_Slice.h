/**
 ******************************************************************************
 * @file    Appli/Core/Inc/venc_h264_config_1080p_Slice.h
 * @author  MCD Application Team
 * @brief   VENC H.264 configuration for 1080p slice mode
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

#ifndef __VENC_H264_CONFIG_1080P_SLICE_H__
#define __VENC_H264_CONFIG_1080P_SLICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Frame dimensions and framerate -------------------------------------------*/
/** Frame height in pixels */
#define VENC_HEIGHT    1080U
/** Frame width in pixels  */
#define VENC_WIDTH     1920U
/** Target framerate (frames per second) */
#define FRAMERATE      15U

/* Hardware mode ------------------------------------------------------------*/
/** Enable hardware handshake / slice / stream mode (1 = enabled) */
#define VENC_HW_MODE_ENABLE     1U

/* Buffer placement ---------------------------------------------------------*/
/** Location macro for VENC internal buffer */
#define VENC_BUFFER_LOCATION    IN_PSRAM
/** Location macro for input frame buffer */
#define INPUT_FRAME_LOCATION    IN_RAM

#ifdef __cplusplus
}
#endif

#endif /* __VENC_H264_CONFIG_1080P_SLICE_H__ */


