/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_audio_play.h
  * @author  MCD Application Team
  * @brief   USBX Device audio plyaback applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UX_DEVICE_AUDIO_PLAY_H__
#define __UX_DEVICE_AUDIO_PLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_audio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "ux_device_class_audio20.h"
#include "ux_device_descriptors.h"
#include "stm32n6570_discovery_audio.h"
#include "ux_device_audio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_AUDIO_PlaybackStreamChange(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                     ULONG alternate_setting);
VOID USBD_AUDIO_PlaybackStreamFrameDone(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                        ULONG length);
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber(VOID);
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize(VOID);
/* USER CODE BEGIN EFP */
UINT USBD_AUDIO_PlaybackInit(VOID);
UINT AUDIO_SpeakerInit(VOID);
VOID AUDIO_SpeakerInitInjectionsParams(VOID);
VOID usbx_audio_play_app_thread(ULONG arg);

#ifdef UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT
uint32_t Audio_get_feedback(VOID);
int16_t USBD_GetSpeedFormat(uint8_t speed, uint32_t rate, uint8_t *FeedBuf);
#endif /*UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT*/
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Size of audio buffer*/
#define AUDIO_TOTAL_BUF_SIZE   192*600/2

#define USB_AUDIO_BUFFER_Filled_SIZE(buffer)    ((buffer.wr_ptr) >= (buffer.rd_ptr)? (buffer.wr_ptr) - (buffer.rd_ptr) : (AUDIO_TOTAL_BUF_SIZE) + (buffer.wr_ptr) - (buffer.rd_ptr))

#define AUDIO_SPEAKER_MAX_INJECTION_LENGTH(audio_desc)\
                                           (((audio_desc).audio_frequency == USBD_AUDIO_FREQ_44_1_K)?AUDIO_SPEAKER_INJECTION_LENGTH(audio_desc) +\
                                             ((audio_desc).audio_channels_count)*((audio_desc).audio_resolution): AUDIO_SPEAKER_INJECTION_LENGTH(audio_desc))

#define AUDIO_SPEAKER_INJECTION_LENGTH(audio_desc) ((audio_desc.audio_frequency)/8000) * \
                                                    (audio_desc.audio_channels_count) * 4

#define AUDIO_SAMPLE_LENGTH(audio_desc) ( (audio_desc).audio_channels_count*(audio_desc).audio_resolution)
#define SPEAKER_ALT_BUFFER_SIZE         ((USBD_AUDIO_PLAY_DEFAULT_FREQ+7999)/8000)*2*3*2

typedef enum
{
  AUDIOPLAYER_STOP = 0,
  AUDIOPLAYER_START,
  AUDIOPLAYER_PLAY,
  AUDIOPLAYER_PAUSE,
  AUDIOPLAYER_EOF,
  AUDIOPLAYER_ERROR,
}AUDIOPLAYER_StateTypedef;

typedef struct
{
  uint32_t        volume;
  uint32_t        mute;
  uint32_t        frequency;
  uint16_t        max_packet_length;
  uint16_t        packet_length;
  uint16_t        packet_length_max_44_1;
  uint8_t         injection_44_count;
  uint16_t        injection_size;
  uint8_t         *data;              /* pointer to data, which is going to transmit using DMA to SAI */
  uint16_t        data_size;          /* size of data, which is going to transmit using DMA to SAI */
  uint8_t*        alt_buffer;         /* alternative buffer used  when underrun is produced(no enough data to inject) or when padding should be added */
  uint16_t        alt_buf_half_size;  /* half size of the alternative buffer */
  uint8_t         double_buff;        /* when the padding is needed the double buffering are required. It means that the alt_buff will contain two packet */
  uint8_t         offset ;
  AUDIOPLAYER_StateTypedef state;
}AUDIO_ProcessTypdef ;

typedef enum {
  PLAY_BUFFER_OFFSET_UNKNOWN = 0,
  PLAY_BUFFER_OFFSET_NONE,
  PLAY_BUFFER_OFFSET_HALF,
  PLAY_BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

/* Audio buffer control structure */
typedef struct {
  uint8_t buff[AUDIO_TOTAL_BUF_SIZE];
  BUFFER_StateTypeDef state;   /* empty (none), half, full*/
  uint32_t rd_ptr;
  uint32_t wr_ptr;
  uint32_t fptr;
  uint32_t size;
}AUDIO_OUT_BufferTypeDef;

typedef struct {
  uint16_t dma_remaining;
  uint16_t threshold;
}AUDIO_SynchroParamsTypeDef;
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_AUDIO_PLAY_H__ */
