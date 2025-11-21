/**
******************************************************************************
* @file          clock_config_600MHz_400MHz_Audio.c
* @author        MCD Application Team
* @brief         Source code
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2025) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
clock_config_t clock_config_600MHz_400MHz = {
  
  .powerSupply          = PWR_SOURCE,
  .regulatorVoltage     = PWR_REGULATOR_VOLTAGE_SCALE0,
  .smpsVoltage          = SMPS_VOLTAGE_NOMINAL,
  .cpuClkSource         = RCC_CPUCLKSOURCE_HSI,
  .sysClkSource         = RCC_CPUCLKSOURCE_HSI,
  
  .RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE,
    .HSIState = RCC_HSI_ON,
    .HSEState = RCC_HSE_ON,
    .HSIDiv   = RCC_HSI_DIV1,
    .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
    .PLL1 = {
      .PLLState = RCC_PLL_ON,
      .PLLSource = RCC_PLLSOURCE_HSI,
      .PLLM = 4,
      .PLLN = 75,
      .PLLFractional = 0,
      .PLLP1 = 1,
      .PLLP2 = 1
    },
    .PLL2 = {
      .PLLState = RCC_PLL_ON,
      .PLLSource = RCC_PLLSOURCE_HSI,
      .PLLM = 8,
      .PLLN = 75,
      .PLLFractional = 0,
      .PLLP1 = 1,
      .PLLP2 = 1
    },
    .PLL3 = {
      .PLLState = RCC_PLL_OFF,
    },
    .PLL4 = {
      .PLLState = RCC_PLL_BYPASS,
    }
  },
  .RCC_ClkInitStruct = {
    .ClockType      = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK4,
    .CPUCLKSource   = RCC_CPUCLKSOURCE_IC1,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_IC2_IC6_IC11,
    .AHBCLKDivider  = RCC_HCLK_DIV2,
    .APB1CLKDivider = RCC_APB1_DIV1,
    .APB2CLKDivider = RCC_APB2_DIV1,
    .APB4CLKDivider = RCC_APB4_DIV1,
    .APB5CLKDivider = RCC_APB5_DIV1,
    .IC1Selection = {
      .ClockSelection = RCC_ICCLKSOURCE_PLL1,
      .ClockDivider = 2
    },
    .IC2Selection = {
      .ClockSelection = RCC_ICCLKSOURCE_PLL1,
      .ClockDivider = 3
    },
    .IC6Selection = {
      .ClockSelection = RCC_ICCLKSOURCE_PLL1,
      .ClockDivider = 2
    },
    .IC11Selection = {
      .ClockSelection = RCC_ICCLKSOURCE_PLL1,
      .ClockDivider = 2
    }
  },
  .PeriphClkInit = {
    .PeriphClockSelection = RCC_PERIPHCLK_XSPI2 | RCC_PERIPHCLK_SDMMC2 | RCC_PERIPHCLK_ETH1 | RCC_PERIPHCLK_XSPI1 | RCC_PERIPHCLK_DCMIPP | RCC_PERIPHCLK_CSI | RCC_PERIPHCLK_LTDC,
    .Xspi2ClockSelection  = RCC_XSPI2CLKSOURCE_IC3,
    .Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_IC4,
    .Eth1ClockSelection   = RCC_ETH1CLKSOURCE_IC12,
    .Xspi1ClockSelection  = RCC_XSPI1CLKSOURCE_HCLK,
    .LtdcClockSelection   = RCC_LTDCCLKSOURCE_IC16,
    .DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17,
   .ICSelection = {
      [RCC_IC3] = {
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider = 6
      },
      [RCC_IC4] = {
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider = 6
      },
      [RCC_IC12] = {
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider = 12
      },
      [RCC_IC16] = {
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider = 30
      },         
      [RCC_IC17] = {
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider = 4   /* 800/3=266.6Mhz */
      },         
       [RCC_IC18] = { // RCC_PERIPHCLK_CSI
        .ClockSelection = RCC_ICCLKSOURCE_PLL1,
        .ClockDivider =60  /* example 1200/60=20Mhz */
      },
    }
  }
};

