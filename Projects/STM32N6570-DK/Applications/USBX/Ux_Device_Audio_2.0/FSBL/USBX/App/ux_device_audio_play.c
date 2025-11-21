/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_audio_play.c
  * @author  MCD Application Team
  * @brief   USBX Device Video applicative source file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_audio_play.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern TX_QUEUE    ux_app_MsgQueue;

#ifdef UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT
AUDIO_SynchroParamsTypeDef             AudioSynchro_params;
uint16_t first_SOF_recieved;
extern SAI_HandleTypeDef               haudio_out_sai;
static UCHAR* audio_feedback_frame;
uint8_t feedback_buffer[4];
#endif /*UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT*/

/* Set BufferCtl start address */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".AudioStreamBufferSection"
#else
__attribute__((section(".AudioStreamBufferSection")))
#endif
/* Double BUFFER for Output Audio stream */
__ALIGN_BEGIN AUDIO_OUT_BufferTypeDef  BufferCtl __ALIGN_END;
AUDIOPLAYER_StateTypedef  AUDIOPLAYER_State;
AUDIO_ProcessTypdef PlaybackAudioProcess;
AUDIO_DescriptionTypeDef PlaybackAudioDescription;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_AUDIO_PlaybackStreamChange
  *         This function is invoked to inform application that the
  *         alternate setting are changed.
  * @param  audio_play_stream: Pointer to audio playback class stream instance.
  * @param  alternate_setting: interface alternate setting.
  * @retval none
  */
VOID USBD_AUDIO_PlaybackStreamChange(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                     ULONG alternate_setting)
{
  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamChange */

  /* Do nothing if alternate setting is 0 (stream closed).  */
  if (alternate_setting == 0)
  {
    return;
  }

  BufferCtl.state = PLAY_BUFFER_OFFSET_UNKNOWN;
  AUDIOPLAYER_State = AUDIOPLAYER_START;

  /* Start reception (stream opened).  */
  ux_device_class_audio_reception_start(audio_play_stream);

  /* USER CODE END USBD_AUDIO_PlaybackStreamChange */

  return;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamFrameDone
  *         This function is invoked whenever a USB packet (audio frame) is received
  *         from the host.
  * @param  audio_play_stream: Pointer to audio playback class stream instance.
  * @param  length: transfer length.
  * @retval none
  */
VOID USBD_AUDIO_PlaybackStreamFrameDone(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                        ULONG length)
{
  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamFrameDone */

  UCHAR         *frame_buffer;
  ULONG         frame_length;

#ifdef UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT
  uint32_t Sample_rate;
#endif /*UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT*/

  /* Get access to first audio input frame.  */
  ux_device_class_audio_read_frame_get(audio_play_stream, &frame_buffer, &frame_length);

  if (length)
  {
    BufferCtl.wr_ptr += frame_length;

    if (BufferCtl.wr_ptr >= ((BufferCtl.size) / 2U))
    {
      BufferCtl.rd_ptr += PlaybackAudioProcess.packet_length;

      if (BufferCtl.rd_ptr >= BufferCtl.size)
      {
        BufferCtl.rd_ptr -= BufferCtl.size;
      }
    }

    if (BufferCtl.wr_ptr >= (AUDIO_TOTAL_BUF_SIZE-100))
    {
      /* All buffers are full: roll back */
      BufferCtl.wr_ptr = 0U;

      if (BufferCtl.state == PLAY_BUFFER_OFFSET_UNKNOWN)
      {

        /* Start BSP play */
        BufferCtl.state = PLAY_BUFFER_OFFSET_NONE;

        /* Put a message queue  */
        if(tx_queue_send(&ux_app_MsgQueue, &BufferCtl.state, TX_NO_WAIT) != TX_SUCCESS)
        {
          Error_Handler();
        }
      }
    }

    ux_utility_memory_copy(&BufferCtl.buff[BufferCtl.wr_ptr], frame_buffer, frame_length);

  }

  /* Re-free the first audio input frame for transfer.  */
  ux_device_class_audio_read_frame_free(audio_play_stream);

#ifdef UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT

  Sample_rate = Audio_get_feedback();

  if (ux_device_class_audio_speed_get(audio_play_stream) == UX_HIGH_SPEED_DEVICE)
    {
      USBD_GetSpeedFormat(UX_HIGH_SPEED_DEVICE, Sample_rate, feedback_buffer);

      audio_feedback_frame = (uint8_t*) feedback_buffer;
    }
  ux_device_class_audio_feedback_set(audio_play_stream, audio_feedback_frame);

#endif /*UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT*/
  /* USER CODE END USBD_AUDIO_PlaybackStreamFrameDone */

  return;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber
  *         This function is invoked to Set audio playback stream max Frame buffer number.
  * @param  none
  * @retval max frame buffer number
  */
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber(VOID)
{
  ULONG max_frame_buffer_number = 0U;

  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber */

  max_frame_buffer_number = 3U;

  /* USER CODE END USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber */

  return max_frame_buffer_number;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize
  *         This function is invoked to Set audio playback stream max Frame buffer size.
  * @param  none
  * @retval max frame buffer size
  */
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize(VOID)
{
  ULONG max_frame_buffer_size = 0U;

  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize */

  max_frame_buffer_size = USBD_AUDIO_PLAY_EPOUT_HS_MPS;

  /* USER CODE END USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize */

  return max_frame_buffer_size;
}

/* USER CODE BEGIN 1 */
/**
  * @brief  USBD_AUDIO_PlaybackInit
  *         Initializes the playback.
  * @param  none
  * @retval status
  */
UINT USBD_AUDIO_PlaybackInit(VOID)
{

  PlaybackAudioDescription.audio_frequency = USBD_AUDIO_PLAY_DEFAULT_FREQ;
  PlaybackAudioDescription.audio_channels_count = USBD_AUDIO_PLAY_CHANNEL_COUNT;
  PlaybackAudioDescription.audio_resolution = USBD_AUDIO_PLAY_RES_BYTE;

  BufferCtl.size = AUDIO_TOTAL_BUF_SIZE;

#if USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K
  AUDIO_SpeakerInit();
#endif /*USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K*/

  PlaybackAudioProcess.packet_length = ((uint32_t)((PlaybackAudioDescription.audio_frequency)/8000)) * \
                                                   (PlaybackAudioDescription.audio_channels_count) * \
                                                   (PlaybackAudioDescription.audio_resolution);

  PlaybackAudioProcess.max_packet_length =((uint32_t)((PlaybackAudioDescription.audio_frequency+7999)/8000)) * \
                                                      (PlaybackAudioDescription.audio_channels_count) * \
                                                      (PlaybackAudioDescription.audio_resolution);


  return UX_SUCCESS;
}

#if USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K

UINT AUDIO_SpeakerInit(VOID)
{
  PlaybackAudioProcess.alt_buffer = ux_utility_memory_allocate(UX_SAFE_ALIGN,
                                                               UX_CACHE_SAFE_MEMORY,
                                                               SPEAKER_ALT_BUFFER_SIZE);

  /* Check Buffer allocated */
  if (PlaybackAudioProcess.alt_buffer == UX_NULL)
  {
    while(1){}
  }

  AUDIO_SpeakerInitInjectionsParams();
  return UX_SUCCESS;
}

/**
  * @brief  AUDIO_SpeakerInitInjectionsParams
  *         audio speaker injection parameter Initialize.
  * @param  none
  * @retval none
  */
VOID AUDIO_SpeakerInitInjectionsParams(VOID)
{
  PlaybackAudioProcess.injection_size = AUDIO_SPEAKER_INJECTION_LENGTH(PlaybackAudioDescription);
  PlaybackAudioProcess.double_buff = 0;
  PlaybackAudioProcess.offset = 0;

  PlaybackAudioProcess.double_buff =1;
  PlaybackAudioProcess.packet_length_max_44_1 = PlaybackAudioProcess.packet_length + AUDIO_SAMPLE_LENGTH(PlaybackAudioDescription);
  PlaybackAudioProcess.alt_buf_half_size = AUDIO_SPEAKER_MAX_INJECTION_LENGTH(PlaybackAudioDescription);

  ux_utility_memory_set(PlaybackAudioProcess.alt_buffer, 0, PlaybackAudioProcess.injection_size);

  /* Start injection of dumped data */
  PlaybackAudioProcess.data = PlaybackAudioProcess.alt_buffer;
  PlaybackAudioProcess.data_size = PlaybackAudioProcess.injection_size;
}

#endif /*USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K*/

/**
  * @brief  Function implementing usbx_audio_play_app_thread.
  * @param arg: Not used
  * @retval None
  */
VOID usbx_audio_play_app_thread(ULONG arg)
{

  while (1)
  {
    /* Wait for a audio device to be connected */
    if (tx_queue_receive(&ux_app_MsgQueue, &BufferCtl.state, TX_WAIT_FOREVER)!= TX_SUCCESS)
    {
      Error_Handler();
    }

    switch(BufferCtl.state)
    {

      case PLAY_BUFFER_OFFSET_NONE:

        BSP_AUDIO_OUT_Play(0, (uint8_t*)&BufferCtl.buff[0], AUDIO_TOTAL_BUF_SIZE);

#if USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K
        if (PlaybackAudioProcess.injection_44_count < 9)
        {
          PlaybackAudioProcess.injection_44_count++;
        }
        else
        {
          PlaybackAudioProcess.injection_44_count = 0;
          PlaybackAudioProcess.data_size = PlaybackAudioProcess.alt_buf_half_size;
        }

        PlaybackAudioProcess.data = &BufferCtl.buff[BufferCtl.rd_ptr];

        uint16_t d = BufferCtl.size - BufferCtl.rd_ptr;
        if (d < PlaybackAudioProcess.data_size)
        {
          ux_utility_memory_copy(PlaybackAudioProcess.alt_buffer, &BufferCtl.buff[BufferCtl.rd_ptr], d);
          ux_utility_memory_copy(PlaybackAudioProcess.alt_buffer + d, &BufferCtl.buff[0], PlaybackAudioProcess.data_size - d);
          PlaybackAudioProcess.data = PlaybackAudioProcess.alt_buffer;
        }
#endif /*USBD_AUDIO_PLAY_FREQ_MAX == USBD_AUDIO_FREQ_44_1_K*/

        break;

    default:
      tx_thread_sleep(MS_TO_TICK(10));
      break;
    }
  }
}

#ifdef UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT

/*this function handles the case of overrun and underrun */
uint32_t Audio_get_feedback(VOID)
{
  uint32_t filled_size;
  int32_t sampleOffset;
  int32_t curFrequency;
  uint32_t feedback;


  /* compute the filled size in the buffer*/
  filled_size = USB_AUDIO_BUFFER_Filled_SIZE(BufferCtl);

  /* compare the filled size with the state of the buffer */
  if (filled_size >= (AUDIO_TOTAL_BUF_SIZE- (AUDIO_TOTAL_BUF_SIZE / 4U)))
  {
    /*case of an overrun*/
    sampleOffset = filled_size - (AUDIO_TOTAL_BUF_SIZE - (AUDIO_TOTAL_BUF_SIZE / 4U));
    sampleOffset /= (PlaybackAudioDescription.audio_channels_count * PlaybackAudioDescription.audio_resolution);
    curFrequency = (PlaybackAudioDescription.audio_frequency) - (sampleOffset * 1000);

    feedback = (uint32_t) ((float)(curFrequency / 8000));
    return feedback;

  }

  if (filled_size < (BufferCtl.size / 4U))
  {
    /* case of an underrun */
    sampleOffset = filled_size;
    sampleOffset /= (PlaybackAudioDescription.audio_channels_count * PlaybackAudioDescription.audio_resolution);
    curFrequency = (PlaybackAudioDescription.audio_frequency) - (sampleOffset *1000);
    feedback = (uint32_t) ((float)(curFrequency / 8000));
    return feedback;
  }

  else
  {
    /* normal case */
    feedback = (uint32_t) ((float)((PlaybackAudioDescription.audio_frequency) / 8000));
    return feedback;
  }
}


int16_t USBD_GetSpeedFormat(uint8_t speed, uint32_t rate, uint8_t *FeedBuf)
{
  uint32_t data_rate;

  /*convert a sample rate into usb high speed format (16.16 format) with k=13 */
  if (speed == UX_HIGH_SPEED_DEVICE)
  {
    data_rate = ((rate << 13))/125;

    FeedBuf[0]= data_rate;
    FeedBuf[1]= (data_rate >> 8) & 0xFF;
    FeedBuf[2]= (data_rate >> 16) & 0xFF;
    FeedBuf[3]= (data_rate >> 24) & 0xFF;
  }
  else
  {
    /*convert a sample rate into usb full speed format (from 16.16 to 10.14 format) with k=10 */
    data_rate = ((rate << 10))/125;

    FeedBuf[0]= (data_rate >> 2) & 0xFF;
    FeedBuf[1]= (data_rate >> 10) & 0xFF;
    FeedBuf[2]= (data_rate >> 18) & 0xFF;
  }
  return data_rate;
}
#endif /*UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT*/
/* USER CODE END 1 */
