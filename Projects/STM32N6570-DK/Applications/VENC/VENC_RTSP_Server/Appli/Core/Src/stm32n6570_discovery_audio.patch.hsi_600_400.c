/**
  ******************************************************************************
  * @file    stm32n6570_discovery_audio_uc6.c
  * @author  MCD Application Team
  * @brief   Audio driver helpers for STM32N6570-DK board (clock configuration)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under BSD 3-Clause license. You may obtain a copy
  * of the License in the LICENSE file in the root directory of this software
  * component. If no LICENSE file is provided, the software is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32n6570_discovery_audio.h"
#include "stm32n6570_discovery_bus.h"

/**
 * @brief  Configure MDF1 clock according to sample rate.
 * @param  hmdf       MDF handle (unused here, kept for API compatibility)
 * @param  SampleRate Audio sample rate used to record the audio stream.
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef MX_MDF1_ClockConfig(MDF_HandleTypeDef *hmdf, uint32_t SampleRate)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hmdf);

    HAL_StatusTypeDef        status = HAL_OK;
    RCC_OscInitTypeDef       RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /* Default: no oscillator change except PLL3 used for MDF1 when required */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    RCC_OscInitStruct.PLL1.PLLState  = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL2.PLLState  = RCC_PLL_NONE;

    /* Configure PLL3 as MDF1 clock source (HSI-based) */
    RCC_OscInitStruct.PLL3.PLLState      = RCC_PLL_ON;
    RCC_OscInitStruct.PLL3.PLLSource     = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL3.PLLFractional = 0U;
    RCC_OscInitStruct.PLL3.PLLM         = 8U; /* 64/8 = 8 MHz */

    if ((SampleRate == AUDIO_FREQUENCY_11K) ||
        (SampleRate == AUDIO_FREQUENCY_22K) ||
        (SampleRate == AUDIO_FREQUENCY_44K))
    {
        /* For 11/22/44 kHz use high PLL multiplier and dividers */
        RCC_OscInitStruct.PLL3.PLLN   = 192U; /* 8 * 192 = 1536 MHz */
        RCC_OscInitStruct.PLL3.PLLP1  = 4U;
        RCC_OscInitStruct.PLL3.PLLP2  = 2U;   /* 1536/8 = 192 MHz */
    }
    else if (SampleRate == AUDIO_FREQUENCY_96K)
    {
        /* For 96 kHz */
        RCC_OscInitStruct.PLL3.PLLN   = 172U; /* 8 * 172 = 1376 MHz */
        RCC_OscInitStruct.PLL3.PLLP1  = 2U;
        RCC_OscInitStruct.PLL3.PLLP2  = 1U;   /* 1376/2 = 688 MHz */
    }
    else
    {
        /* For 8/16/32/48 kHz */
        RCC_OscInitStruct.PLL3.PLLN   = 172U; /* 8 * 172 = 1376 MHz */
        RCC_OscInitStruct.PLL3.PLLP1  = 7U;
        RCC_OscInitStruct.PLL3.PLLP2  = 4U;   /* 1376/28 ≈ 49.14 MHz */
    }

    RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        status = HAL_ERROR;
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_MDF1;
    PeriphClkInitStruct.Mdf1ClockSelection   = RCC_MDF1CLKSOURCE_IC8;
    PeriphClkInitStruct.ICSelection[RCC_IC8].ClockSelection = RCC_ICCLKSOURCE_PLL3;

    if ((SampleRate == AUDIO_FREQUENCY_11K) ||
        (SampleRate == AUDIO_FREQUENCY_22K) ||
        (SampleRate == AUDIO_FREQUENCY_44K))
    {
        PeriphClkInitStruct.ICSelection[RCC_IC8].ClockDivider = 17U;
    }
    else if (SampleRate == AUDIO_FREQUENCY_96K)
    {
        PeriphClkInitStruct.ICSelection[RCC_IC8].ClockDivider = 7U;
    }
    else /* 8K, 16K, 32K, 48K */
    {
        PeriphClkInitStruct.ICSelection[RCC_IC8].ClockDivider = 1U;
    }

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        status = HAL_ERROR;
    }

    return status;
}

/**
 * @brief  Configure SAI1 clock according to sample rate.
 * @note   The SAI1 clock configuration uses HSI as the base clock in this platform.
 * @param  hsai       SAI handle (unused here, kept for API compatibility)
 * @param  SampleRate Audio sample rate used to play the audio stream.
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hsai);

    HAL_StatusTypeDef        ret = HAL_OK;
    RCC_OscInitTypeDef       RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /* Assume HSI is running; configure PLL4 when needed */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    RCC_OscInitStruct.PLL1.PLLState  = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL2.PLLState  = RCC_PLL_NONE;
    RCC_OscInitStruct.PLL3.PLLState  = RCC_PLL_NONE;

    RCC_OscInitStruct.PLL4.PLLState      = RCC_PLL_ON;
    RCC_OscInitStruct.PLL4.PLLSource     = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL4.PLLFractional = 0U;
    RCC_OscInitStruct.PLL4.PLLM          = 8U; /* 64/8 = 8 MHz */

    if ((SampleRate == AUDIO_FREQUENCY_11K) ||
        (SampleRate == AUDIO_FREQUENCY_22K) ||
        (SampleRate == AUDIO_FREQUENCY_44K))
    {
        RCC_OscInitStruct.PLL4.PLLN  = 192U; /* 8 * 192 = 1536 MHz */
        RCC_OscInitStruct.PLL4.PLLP1 = 4U;
        RCC_OscInitStruct.PLL4.PLLP2 = 2U;   /* 1536/8 = 192 MHz */
    }
    else
    {
        /* 8K, 16K, 24K, 32K, 48K or 96K */
        RCC_OscInitStruct.PLL4.PLLN  = 172U; /* 8 * 172 = 1376 MHz */
        RCC_OscInitStruct.PLL4.PLLP1 = 7U;
        RCC_OscInitStruct.PLL4.PLLP2 = 4U;   /* 1376/28 ≈ 49.14 MHz */
    }

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        ret = HAL_ERROR;
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    PeriphClkInitStruct.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_IC7;
    PeriphClkInitStruct.ICSelection[RCC_IC7].ClockSelection = RCC_ICCLKSOURCE_PLL4;

    if ((SampleRate == AUDIO_FREQUENCY_11K) ||
        (SampleRate == AUDIO_FREQUENCY_22K) ||
        (SampleRate == AUDIO_FREQUENCY_44K))
    {
        PeriphClkInitStruct.ICSelection[RCC_IC7].ClockDivider = 17U;
    }
    else
    {
        PeriphClkInitStruct.ICSelection[RCC_IC7].ClockDivider = 1U;
    }

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        ret = HAL_ERROR;
    }

    return ret;
}



