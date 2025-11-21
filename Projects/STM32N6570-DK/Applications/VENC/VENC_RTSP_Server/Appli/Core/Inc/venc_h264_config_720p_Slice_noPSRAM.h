/**
 ******************************************************************************
 * @file    Appli/Core/Inc/venc_h264_config_720p_Slice_noPSRAM.h
 * @author  MCD Application Team
 * @brief   VENC H.264 configuration for 720p slice mode (no PSRAM)
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

#ifndef __VENC_H264_CONFIG_720P_SLICE_NO_PSRAM_H__
#define __VENC_H264_CONFIG_720P_SLICE_NO_PSRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Frame dimensions and framerate -------------------------------------------*/
/** Frame height in pixels */
#define VENC_HEIGHT    720U
/** Frame width in pixels  */
#define VENC_WIDTH     1280U
/** Target framerate (frames per second) */
#define FRAMERATE      30U

/* Hardware mode ------------------------------------------------------------*/
/** Enable hardware handshake / slice / stream mode (1 = enabled) */
#define VENC_HW_MODE_ENABLE     1U

/* Buffer placement ---------------------------------------------------------*/
/** Location macro for VENC internal buffer */
#define VENC_BUFFER_LOCATION    IN_RAM
/** Location macro for input frame buffer */
#define INPUT_FRAME_LOCATION    IN_RAM

/* Pixel / format configuration ---------------------------------------------*/
#if 1
/** DCMIPP pixel packer format */
#define DCMIPP_FORMAT                   DCMIPP_PIXEL_PACKER_FORMAT_YUV420_2
/** VENC input pixel format */
#define VENC_INPUT_FORMAT               H264ENC_YUV420_SEMIPLANAR
/** Bytes per pixel (YUV420 = 12 bits per pixel => 1.5 bytes) */
#define DCMIPP_BYTES_PER_PIXELS         (1.5f)
/** Pool size for encoder buffers (bytes) */
#define VENC_POOL_SIZE                  (2306868U)
#else
/** Alternative configuration (YUV422) */
#define DCMIPP_FORMAT                   DCMIPP_PIXEL_PACKER_FORMAT_YUV422_1
#define VENC_INPUT_FORMAT               H264ENC_YUV422_INTERLEAVED_YUYV
#define DCMIPP_BYTES_PER_PIXELS         (2U)            /* YUV422: 16 bits per pixel */
#define VENC_POOL_SIZE                  (2306868U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VENC_H264_CONFIG_720P_SLICE_NO_PSRAM_H__ */


