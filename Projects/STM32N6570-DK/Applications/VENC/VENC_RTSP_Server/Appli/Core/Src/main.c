/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "string.h"
#include "app_threadx.h"
#include "stm32n6570_discovery_xspi.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#if defined(__ICCARM__)
#include <LowLevelIOInterface.h>
#endif /* __ICCARM__ */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined(__ARMCC_VERSION)
/* ARM Compiler 6 */
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ETH_DMADescTypeDef  DMARxDscrTab[ETH_DMA_RX_CH_CNT][ETH_RX_DESC_CNT] __NON_CACHEABLE; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_DMA_TX_CH_CNT][ETH_TX_DESC_CNT] __NON_CACHEABLE; /* Ethernet Tx DMA Descriptors */
ETH_TxPacketConfig  TxConfig;
ETH_HandleTypeDef   heth;
UART_HandleTypeDef  huart1;

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_USART1_UART_Init(void);
static void MPU_Config(void);
static void RISAF_Config(void);
static void XSPI1_Source_Init(void);
static void MX_GPDMA1_Init(void);
void power_default_system_clock_config(void);
void MX_TheadX_Init(void);


/**
  * @brief  The application entry point.
  * @retval int
  */
  
int main(void)
{
  /* MPU Configuration */
  MPU_Config();
  
  /* Enable I-Cache */
  SCB_EnableICache();
  
  /* Enable D-Cache */
  SCB_EnableDCache();

  /* System clock already configured, simply SystemCoreClock init */
  SystemCoreClockUpdate();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
 
  /* Clocks configuration*/
  power_default_system_clock_config();
 
  /*  Risaf configuration */
  RISAF_Config();

  /*  External RAM  Init */
  XSPI1_Source_Init();
  BSP_XSPI_RAM_Init(0);
  BSP_XSPI_RAM_EnableMemoryMappedMode(0);
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART1_UART_Init();
  MX_GPDMA1_Init();

  /* Start OS*/
  MX_ThreadX_Init();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  while (1)
  {
  }
}

static void  XSPI1_Source_Init(void)
{ 
  
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_XSPI1;
  PeriphClkInit.Xspi1ClockSelection = RCC_XSPI1CLKSOURCE_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  return;
}


void H264EncTrace(const char *pMsg)
{
  printf("VENC[%s]\n", pMsg);
}

static void MX_GPDMA1_Init(void)
{
  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */
}


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance        = USART1;
  huart1.Init.BaudRate   = 115200;
  huart1.Init.Mode       = UART_MODE_TX_RX;
  huart1.Init.Parity     = UART_PARITY_NONE;
  huart1.Init.StopBits   = UART_STOPBITS_1;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH1;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE0;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x10;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RGMII_MODE;

  for (int ch = 0; ch < ETH_DMA_CH_CNT; ch++)
  {
    heth.Init.TxDesc[ch] = DMATxDscrTab[ch];
    heth.Init.RxDesc[ch] = DMARxDscrTab[ch];
  }

  heth.Init.RxBuffLen = 1536;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_YELLOW_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
#if defined(__ICCARM__)
size_t __write(int file, unsigned char const *ptr, size_t len)
{
  size_t idx;
  unsigned char const *pdata = ptr;

  for (idx = 0; idx < len; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
  return len;
}
#endif /* __ICCARM__ */
/**
  * @brief  Retargets the C library printf function to the USART.
  */

#ifndef TERMINAL_IO
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of putchar here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
#endif
/* USER CODE END 4 */

/**
* @brief  RISAF Configuration.
* @retval None
*/
static void RISAF_Config(void)
{
  __HAL_RCC_RIFSC_CLK_ENABLE();
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_ETH1, &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1 , &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC2 , &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_VENC  , &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D  , &RIMC_master);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ETH1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_CSI    , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_VENC   , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDC   , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

  /* AXISRAM1 full secure */
  RISAF2_S->REG[0].STARTR  = 0x0;
  RISAF2_S->REG[0].ENDR    = 0x000FFFFF;  /* Al SRAM1 in base region (1Mbytes) */
  RISAF2_S->REG[0].CIDCFGR = 0x00030003;  /* RW by DMA (CID 0) and CPU (CID 1) */
  RISAF2_S->REG[0].CFGR    = 0x101;       /* Base region set with SEC attribute */

  RISAF3_S->REG[0].STARTR  = 0x0;
  RISAF3_S->REG[0].ENDR    = 0x000FFFFF;  /* Al SRAM1 in base region (1Mbytes) */
  RISAF3_S->REG[0].CIDCFGR = 0x00030003;  /* RW by DMA (CID 0) and CPU (CID 1) */
  RISAF3_S->REG[0].CFGR    = 0x101;       /* Base region set with SEC attribute */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  printf("Critical error has occurred\r\n");
  while (1)
  {
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port,LED_RED_Pin);
    HAL_Delay(1000);
  }
  /* USER CODE END Error_Handler_Debug */
}
static void MPU_Config(void)
{
  uint32_t primask_bit = __get_PRIMASK();
  MPU_Region_InitTypeDef default_config = {0};
  MPU_Attributes_InitTypeDef attr_config = {0};
  uint32_t region_number = MPU_REGION_NUMBER0;
  __disable_irq();

  /* disable the MPU */
  HAL_MPU_Disable();

  /* create an attribute configuration for the MPU */
  attr_config.Number = MPU_ATTRIBUTES_NUMBER0;
  attr_config.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
  HAL_MPU_ConfigMemoryAttributes(&attr_config);

  attr_config.Number = MPU_ATTRIBUTES_NUMBER1;
  attr_config.Attributes = INNER_OUTER( MPU_WRITE_THROUGH|MPU_NON_TRANSIENT|MPU_RW_ALLOCATE);
  HAL_MPU_ConfigMemoryAttributes(&attr_config);

#define UNCACHED_ATTRIBUTE MPU_ATTRIBUTES_NUMBER0
#define CACHED_ATTRIBUTE   MPU_ATTRIBUTES_NUMBER1
  
  /* Define RO */
  default_config.Enable           = MPU_REGION_ENABLE;
  default_config.Number           = region_number++;
  default_config.AttributesIndex  = CACHED_ATTRIBUTE; /*Cached*/
  default_config.BaseAddress      = 0x34000000;
  default_config.LimitAddress     = 0x340963FF;
  default_config.AccessPermission = MPU_REGION_ALL_RO;
  default_config.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  default_config.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&default_config);

   /* Define Not Cacheable area */
  default_config.Enable           = MPU_REGION_ENABLE;
  default_config.Number           = region_number++;
  default_config.AttributesIndex  = UNCACHED_ATTRIBUTE;  /*uncached*/
  default_config.BaseAddress      = 0x34096400;
  default_config.LimitAddress     = 0x341163FF;
  default_config.AccessPermission = MPU_REGION_ALL_RW;
  default_config.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  default_config.IsShareable      = MPU_ACCESS_INNER_SHAREABLE | MPU_ACCESS_OUTER_SHAREABLE;
  HAL_MPU_ConfigRegion(&default_config);


   /* Define RW */
  default_config.Enable           = MPU_REGION_ENABLE;
  default_config.Number           = region_number++;
  default_config.AttributesIndex  = CACHED_ATTRIBUTE;  /*Cached*/
  default_config.BaseAddress      = 0x34116400;
  default_config.LimitAddress     = 0x34AC33FF;
  default_config.AccessPermission = MPU_REGION_ALL_RW;
  default_config.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  default_config.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&default_config);
 
 

   /* Define external RAM Cacheable area */
  default_config.Enable           = MPU_REGION_ENABLE;
  default_config.Number           = region_number++;
  default_config.AttributesIndex  = CACHED_ATTRIBUTE;  /*Cached*/
  default_config.BaseAddress      = 0x90000000;
  default_config.LimitAddress     = 0x91ffffff;
  default_config.AccessPermission = MPU_REGION_ALL_RW;
  default_config.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  default_config.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&default_config);
 

  /* enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  __enable_irq();
  /* Exit critical section to lock the system and avoid any issue around MPU mechanisme */
  __set_PRIMASK(primask_bit);
}
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
