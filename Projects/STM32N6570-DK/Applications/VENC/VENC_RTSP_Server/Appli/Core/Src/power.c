/**
******************************************************************************
* @file          power.c
* @author        MCD Application Team
* @brief         Power management
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "stdio.h"
#include "clock_conf.h"
#include "power.h"
#include "string.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_xspi.h"
#include "stm32n6570_discovery_sd.h"

#include "clock_config_600MHz_400MHz_Audio.c"

#ifndef  POWER_TRACE_INFO
#define POWER_TRACE_INFO(...)  {printf(__VA_ARGS__);printf("\n");}

#endif

void Error_Handler(void);


/**
* @brief  Configure clock tree according to parameter
* @param  clock_config: clock tree configuration.
* @retval None.
*/
static void s_power_generic_system_clock_config(clock_config_t *clock_config);
static void s_power_generic_system_clock_config(clock_config_t *clock_config)
{

  /* Configure Power Supply */
  if (HAL_PWREx_ConfigSupply(clock_config->powerSupply) != HAL_OK)
  {
     Error_Handler();
  }
  
  BSP_SMPS_Init(clock_config->smpsVoltage);

  
  __HAL_RCC_CPUCLK_CONFIG(RCC_CPUCLKSOURCE_HSI);
  __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);
  
  /* Control Voltage Scaling */
  if (HAL_PWREx_ControlVoltageScaling(clock_config->regulatorVoltage) != HAL_OK)
  {
     Error_Handler();
  }
  
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  
  /* Switch Back to HSI if needed */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct);
  if ((RCC_ClkInitStruct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
      (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11))
  {
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK);
    RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
       Error_Handler();
    }
  }
  
  /* Configure Oscillator according to Clock parameters */
  if (HAL_RCC_OscConfig(&clock_config->RCC_OscInitStruct) != HAL_OK)
  {
     Error_Handler();
  }
  
  /* Configure main clocks according to Clock parameters */
  if (HAL_RCC_ClockConfig(&clock_config->RCC_ClkInitStruct) != HAL_OK)
  {
     Error_Handler();
  }
  
  /* Configure peripheral clocks according to Clock parameters */
  if (HAL_RCCEx_PeriphCLKConfig(&clock_config->PeriphClkInit) != HAL_OK)
  {
     Error_Handler();
  }
}

/**
* @brief  Configure clock tree according to predefined configuration
* @param  config: clock tree configuration.
* @retval None.
*/
void power_alternate_system_clock_config(power_config_t config)
{
  clock_config_t *clock_config;
  bool b_clock_config = true;
  
  if (config & POWER_CONFIG_600MHZ_400MHZ) 
  {
    clock_config = &clock_config_600MHz_400MHz;
  } 
  else 
  {
    /* This power config is not a clock config */
    b_clock_config = false;
  }
  
  if (b_clock_config)
  {
    s_power_generic_system_clock_config(clock_config);
  }
}

/**
* @brief  Configure clock tree according to default configuration
* @retval None.
*/
void power_default_system_clock_config(void)
{
  power_alternate_system_clock_config(POWER_CLOCK_DEFAULT_CONFIG);
}




