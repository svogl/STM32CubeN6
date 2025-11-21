/**
******************************************************************************
* @file          plugin_audio.c
* @author        MCD Application Team
* @brief         Encoder support (audio capture and callbacks)
******************************************************************************
* @attention
*
* Copyright (c) 2025 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "stm32n6570_discovery_audio.h"
#include "app_rtsp_over_rtp.h"

/* Private defines -----------------------------------------------------------*/
#define AUDIO_FREQUENCY         AUDIO_FREQUENCY_16K
#define AUDIO_CAPTURE_IT_MS     20U
#define NB_MICS                 1U
/* Number of 16-bit samples in capture buffer (double-buffer: half + full) */
#define CAPTURE_BUFFER_SIZE     (2U * (NB_MICS * AUDIO_CAPTURE_IT_MS * AUDIO_FREQUENCY / 1000U))

/* Audio in buffer - placed in non-cached memory */
int16_t PCM_Buffer[CAPTURE_BUFFER_SIZE];

/* Frame pointer and counters shared with ISR -> mark volatile */
static volatile int16_t *pcm_frame;
static volatile uint32_t pcm_frame_number;
static volatile uint32_t last_pcm_frame_number;

/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);

/* Public API ----------------------------------------------------------------*/

/**
 * @brief  Initialize audio capture hardware.
 * @note   Uses BSP audio driver for digital microphone input.
 * @retval BSP status code (BSP_ERROR_NONE on success)
 */
int32_t audio_init(void)
{
  uint32_t audioState;
  int32_t err;
  BSP_AUDIO_Init_t AudioInit;

  /* Test audio input state */
  err = BSP_AUDIO_IN_GetState(1, &audioState);
  if (err != BSP_ERROR_NONE)
  {
    printf("BSP_AUDIO_IN_GetState failed !!\n");
    return err;
  }

  if (audioState != AUDIO_IN_STATE_RESET)
  {
    printf("audioState != AUDIO_IN_STATE_RESET !!!\n");
    return BSP_ERROR_BUSY;
  }

  AudioInit.Device        = AUDIO_IN_DEVICE_DIGITAL_MIC;
  AudioInit.SampleRate    = AUDIO_FREQUENCY;
  AudioInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioInit.ChannelsNbr   = NB_MICS;
  AudioInit.Volume        = 80; /* Not used */

  err = BSP_AUDIO_IN_Init(1, &AudioInit);
  if (err != BSP_ERROR_NONE)
  {
    printf("BSP_AUDIO_IN_Init failed !!\n");
  }

  return err;
}

/**
 * @brief  Start audio capture (record).
 * @retval BSP status code (BSP_ERROR_NONE on success)
 */
int32_t audio_start(void)
{
  int32_t err;

  pcm_frame_number = 0;
  last_pcm_frame_number = 0;

  /* Start record. Buffer length in bytes */
  err = BSP_AUDIO_IN_Record(1, (uint8_t *)PCM_Buffer, CAPTURE_BUFFER_SIZE * sizeof(int16_t));
  if (err != BSP_ERROR_NONE)
  {
    printf("BSP_AUDIO_IN_Record failed !!\n");
  }

  return err;
}

/**
 * @brief  Stop audio capture.
 * @retval BSP status code
 */
int32_t s_audio_stop(void)
{
  int32_t err;

  err = BSP_AUDIO_IN_Stop(1);
  return err;
}

/**
 * @brief  Minimal test helper to init and start audio capture.
 */
void test_audio(void)
{
  audio_init();
  HAL_Delay(1000);
  audio_start();
}

/* IRQ Handler ---------------------------------------------------------------*/

/**
 * @brief  GPDMA channel IRQ handler forwards to BSP audio IRQ handler.
 */
void GPDMA1_Channel0_IRQHandler(void)
{
  BSP_AUDIO_IN_IRQHandler(1, AUDIO_IN_DEVICE_DIGITAL_MIC);
}

/* BSP audio callbacks ------------------------------------------------------*/

/**
 * @brief  BSP audio-in transfer complete callback.
 * @param  Instance  Audio instance (expected 1)
 * @retval None
 */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    /* Point to second half of the double buffer */
    pcm_frame = PCM_Buffer + (CAPTURE_BUFFER_SIZE / 2);
    pcm_frame_number++;
    tx_event_flags_set(&demo_test_events, DEMO_AUDIO_DATA_READY_EVENT, TX_OR);
  }
}

/**
 * @brief  BSP audio-in half-transfer complete callback.
 * @param  Instance  Audio instance (expected 1)
 * @retval None
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    /* Point to first half of the double buffer */
    pcm_frame = PCM_Buffer;
    pcm_frame_number++;
    tx_event_flags_set(&demo_test_events, DEMO_AUDIO_DATA_READY_EVENT, TX_OR);
  }
}

/**
 * @brief  BSP audio-in error callback.
 * @param  Instance  Audio instance
 * @retval None
 */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
  (void)Instance;
  printf("BSP_AUDIO_IN_Error_CallBack\n");
}

/* Application helpers ------------------------------------------------------*/

/**
 * @brief  Get pointer to the last ready PCM data and its size in bytes.
 * @param  data  Output pointer to PCM buffer (points to half-buffer region)
 * @param  size  Output size in bytes
 * @retval 0
 *
 * @note   This function checks for buffer overflow (producer/consumer mismatch).
 */
uint32_t AUDIO_APP_GetData(uint8_t **data, size_t *size)
{
  uint32_t current_pcm_frame_number = pcm_frame_number;
  
  if (++last_pcm_frame_number != current_pcm_frame_number)
  {
    printf("AUDIO CAPTURE overflow !!!\n");
  }

  *data = (uint8_t *)pcm_frame;
  *size = (CAPTURE_BUFFER_SIZE / 2) * sizeof(int16_t);
  last_pcm_frame_number = current_pcm_frame_number;
  return 0U;
}

/**
 * @brief  Return the last PCM frame sequence number produced by ISR.
 * @retval pcm frame sequence number
 */
uint32_t AUDIO_APP_LastPcmFrameNumber(void)
{
  return pcm_frame_number;
}

