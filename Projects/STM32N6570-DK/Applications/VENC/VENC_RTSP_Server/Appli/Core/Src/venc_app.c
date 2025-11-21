/*------------------------------------------------------------------------------
 * @file    Appli/Core/Src/venc_app.c
* @author  MCD Application Team
 * @brief   Video encoder application (thread, encoder control and buffers)
 *
* @attention
*
 * Copyright (c) 2025 STMicroelectronics.
* All rights reserved.
*
 * This software component is licensed by ST under BSD 3-Clause license.
 *------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "ewl.h"
#include "H264EncApi.h"
#include "venc_app.h"
#include "stlogo.h"
#include "imx335.h"
#include "stm32n6xx_ll_venc.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_lcd.h"
#include "stm32n6570_discovery_camera.h"
#include "tx_api.h"
#include "nx_api.h"
#include "app_rtsp_over_rtp.h"
#include "app_netxduo.h"
#include "utils.h"
#include "venc_h264_config.h"

/** @addtogroup Templates
* @{
*/

/** @addtogroup HAL
* @{
*/

/* Private typedef -----------------------------------------------------------*/
typedef struct {
  uint32_t size;
  uint32_t * block_addr;
  uint32_t * aligned_block_addr;
} venc_output_frame_t;
/* Private define ------------------------------------------------------------*/
/* Align and use unsigned suffixes for sizes/counts */
#define VENC_APP_QUEUE_SIZE        15U
#define VENC_OUTPUT_BLOCK_SIZE     (100U * 1024U)
#define VENC_OUTPUT_BLOCK_NBR      4U

/* Private macro -------------------------------------------------------------*/
/* Align a pointer up to 'bytes' boundary */
#define ALIGNED(ptr, bytes)       ((((uintptr_t)(ptr) + ((bytes) - 1U)) / (bytes)) * (bytes))

/* Private variables ---------------------------------------------------------*/


extern DCMIPP_HandleTypeDef hcamera_dcmipp;


static H264EncIn encIn= {0};
static H264EncOut encOut= {H264ENC_INTRA_FRAME,0, 0, 0, 0, 0, 0, 0,  H264ENC_NO_REFERENCE_NO_REFRESH,  H264ENC_NO_REFERENCE_NO_REFRESH};
static H264EncInst encoder= {0};
static uint32_t frame_nb = 0;
static uint32_t frame_received = 0;
static uint32_t last_frame_received = 0;
uint32_t nbLineEvent=0;
/* Input Frame : in internal ram */
venc_output_frame_t queue_buf[VENC_OUTPUT_BLOCK_NBR];
uint8_t output_block_buffer[VENC_OUTPUT_BLOCK_NBR * VENC_OUTPUT_BLOCK_SIZE] ALIGN_32 __NON_CACHEABLE ;

TX_EVENT_FLAGS_GROUP venc_app_flags;
TX_QUEUE enc_frame_queue;
TX_BLOCK_POOL venc_block_pool;

/* Private function prototypes -----------------------------------------------*/
static int encoder_prepare(void);
static int encode_frame(void);
static int encoder_end(void);
static int encoder_start(void);

/**
  * @brief  Checks if a video buffer overflow condition has occurred.
  * @note   This function is typically used to monitor the video streaming process
  *         and detect if the video buffer has exceeded its capacity, which may
  *         result in data loss or corruption.
  * @retval true  if a video overflow condition is detected.
  * @retval false if no overflow condition is present.
  *
  */
bool IsVideoOverflow(void)    
{
    bool videoOverflow = false;

    if (IsHwHanshakeMode())
    {
        videoOverflow = (nbLineEvent != 0U);
    }
    else
    {
        /* Ping-pong buffer overflow detection */
        videoOverflow = (frame_received > last_frame_received + 1U);
        last_frame_received = frame_received;
    }
    return videoOverflow;
}

/**
 * @brief  VENC application thread function.
 * @param  arg Thread argument (unused).
 *
 * Waits for VIDEO_START_FLAG; processes camera background tasks and, for each
 * FRAME_RECEIVED_FLAG, encodes the frame and queues the output for transmission.
 */
void venc_thread_func(ULONG arg)
{
    ULONG flags;
    uint32_t x_size;
    uint32_t y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);
    /* initialize LEDs to signal processing is ongoing */
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);

    if(tx_event_flags_create(&venc_app_flags, "venc_app_events") != TX_SUCCESS)
    {
        return ;
    }
    if(tx_queue_create(&enc_frame_queue, "ENC frame queue", sizeof(venc_output_frame_t)/4, &queue_buf, sizeof(queue_buf)) != TX_SUCCESS)
    {
        Error_Handler();
    }

    if(tx_block_pool_create(&venc_block_pool, "venc output bock pool", VENC_OUTPUT_BLOCK_SIZE,
                            output_block_buffer, VENC_OUTPUT_BLOCK_NBR * VENC_OUTPUT_BLOCK_SIZE) != TX_SUCCESS)
    {
        Error_Handler();
    }

     /* Initialize camera */
    if(BSP_CAMERA_Init(0,CAMERA_R2592x1944, CAMERA_PF_RAW_RGGB10) != BSP_ERROR_NONE)
    {
        Error_Handler();
    }

    IMX335_SetFramerate(Camera_CompObj, hVencH264Instance.cfgH264Main.frameRateNum /hVencH264Instance.cfgH264Main.frameRateDenom);
    /* initialize VENC */
    LL_VENC_Init();

    /* initialization done. Turn on the LEDs */
    BSP_LED_On(LED1);
    BSP_LED_On(LED2);

    if (encoder_prepare())
    {
        printf("encoder_prepare failed !!\n");
    }

    while(1)
    {
        tx_event_flags_get(&venc_app_flags, VIDEO_START_FLAG, TX_AND, &flags, TX_WAIT_FOREVER);
        if(BSP_CAMERA_BackgroundProcess() != BSP_ERROR_NONE)
        {
            printf("Error in BSP image processing\n");
        }
        tx_event_flags_get(&venc_app_flags, FRAME_RECEIVED_FLAG, TX_AND_CLEAR, &flags, TX_WAIT_FOREVER);
        if (IsVideoOverflow())
        {
            printf("Video Overflow - Skip Frame\n");
            continue; 
        }
        if(encode_frame())
        {
            printf("error encoding frame\n");
        }
        else
        {
            BSP_LED_Toggle(LED_GREEN);
            tx_event_flags_set(&demo_test_events, DEMO_VIDEO_DATA_READY_EVENT, TX_OR);
        }
    }
}

/**
 * @brief  Prepare encoder: initialize encoder instance and configure
 *         preprocessing, coding control and rate control parameters.
 * @retval int 0 on success, -1 on failure
 */
static int encoder_prepare(void)
{
    H264EncRet ret;

    frame_nb = 0U;

    /* Set encode configuration */
    ret = H264EncInit(&hVencH264Instance.cfgH264Main, &encoder);
    if (ret != H264ENC_OK)
    {
        return -1;
    }

    /* Set preprocessing */
    ret = H264EncSetPreProcessing(encoder, &hVencH264Instance.cfgH264Preproc);
    if (ret != H264ENC_OK)
    {
        return -1;
    }

    ret = H264EncSetCodingCtrl(encoder, &hVencH264Instance.cfgH264Coding);
    if (ret != H264ENC_OK)
    {
        return -1;
    }

    /* Set rate control */
    ret = H264EncSetRateCtrl(encoder, &hVencH264Instance.cfgH264Rate);
    if (ret != H264ENC_OK)
    {
        return -1;
    }

    return 0;
  }

/**
 * @brief Resets the encoder to its initial state.
 *
 * This function performs all necessary operations to bring the encoder hardware
 * and associated software state back to a known, default condition. It should be
 * called whenever a full reinitialization of the encoder is required, such as after
 * an error or before starting a new encoding session.
 *
 * @note This function is static and intended for internal use within this module only.
 */
static void  encoder_reset(void)
  {
  if (HAL_DCMIPP_PIPE_Suspend(&hcamera_dcmipp, DCMIPP_PIPE1) != HAL_OK)
  {
    printf("HAL_DCMIPP_PIPE_Suspend failed\n");
  }
  /* Wait for end of trame */
  tx_thread_sleep(15U * TX_TIMER_TICKS_PER_SECOND / 1000U);

  
  /* VENV HW REset */
  __HAL_RCC_VENC_FORCE_RESET();
  tx_thread_sleep(1U * TX_TIMER_TICKS_PER_SECOND / 1000U);
  __HAL_RCC_VENC_RELEASE_RESET();
  tx_thread_sleep(1U * TX_TIMER_TICKS_PER_SECOND / 1000U);
  
  /*Resume DCMIPP after VENC reset*/
  if (HAL_DCMIPP_PIPE_Resume(&hcamera_dcmipp, DCMIPP_PIPE1) != HAL_OK)
  {
    printf("HAL_DCMIPP_PIPE_Resume failed");
  }
}


static DCMIPP_FullPlanarDstAddressTypeDef dcmipp_address;

/**
 * @brief Configures the DCMIPP (Digital Camera Memory Interface Pixel Processor) peripheral.
 *
 * This function initializes and sets up the DCMIPP hardware for video capture or processing.
 * It typically configures input formats, resolutions, and other relevant parameters required
 * for proper operation of the DCMIPP in the application.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 */
static int dcmipp_config(void)
{
    int ret = 0;

    /* One Input Buffer : YUYV ... YUYV */
    if (GetDCMIPPFormat() == DCMIPP_PIXEL_PACKER_FORMAT_YUV422_1)
    {
        if (hVencH264Instance.cfgH264Preproc.inputType != H264ENC_YUV422_INTERLEAVED_YUYV)
        {
            printf("Input Frame inconsistency : DCMIPP_PIXEL_PACKER_FORMAT_YUV422_1 vs %d\n", hVencH264Instance.cfgH264Preproc.inputType);
            return -1;
        }

        dcmipp_address.YAddress = (uint32_t)GetInputFrame(NULL);
        dcmipp_address.UAddress = dcmipp_address.YAddress;
        dcmipp_address.VAddress = dcmipp_address.UAddress;

        ret = BSP_CAMERA_Start(0, GetInputFrame(NULL), CAMERA_MODE_CONTINUOUS);
    }
    /* Two Input Buffer : Y...Y  UV...UV */
    else if (GetDCMIPPFormat() == DCMIPP_PIXEL_PACKER_FORMAT_YUV420_2)
    {
        DCMIPP_SemiPlanarDstAddressTypeDef dcmipp_semi_planar_address;
        
        if (hVencH264Instance.cfgH264Preproc.inputType != H264ENC_YUV420_SEMIPLANAR)
        {
            printf("Input Frame inconsistency : DCMIPP_PIXEL_PACKER_FORMAT_YUV420_2 vs %d\n", hVencH264Instance.cfgH264Preproc.inputType);
            return -1;
        }
           
        /* fill dcmipp_address for VENC paarameters*/
        dcmipp_address.YAddress = (uint32_t)GetInputFrame(NULL);
        dcmipp_address.UAddress = dcmipp_address.YAddress  + hVencH264Instance.cfgH264Main.width *  GetDCMIPPNbLinesCaptured();
        dcmipp_address.VAddress = dcmipp_address.UAddress;
        
        dcmipp_semi_planar_address.YAddress  = dcmipp_address.YAddress;
        dcmipp_semi_planar_address.UVAddress = dcmipp_address.UAddress;
        
        ret = BSP_CAMERA_SemiPlanarStart(0, &dcmipp_semi_planar_address,CAMERA_MODE_CONTINUOUS);
    }
    /* Three Input Buffer : unsupported */
    else
    {
        printf("Unsupported DCMIPP format\n");
        return -1;
    }

    /* Enable multiline interrupt: needed for encode pace check */
    __HAL_DCMIPP_ENABLE_IT(&hcamera_dcmipp, DCMIPP_IT_PIPE1_LINE);

    return ret;
}

/**
 * @brief Starts the video encoder.
 *
 * This function initializes and starts the video encoder process.
 * It prepares all necessary resources and configurations required
 * for encoding video streams.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 */
static int encoder_start(void)
{
    H264EncRet ret;
    venc_output_frame_t frame_buffer = {0};

  
  if (dcmipp_config())
  {
    return -1;
  }
  
  if(tx_block_allocate(&venc_block_pool, (void **) &frame_buffer.block_addr, TX_NO_WAIT) != TX_SUCCESS)
  {
    return -1;
  }
  frame_buffer.aligned_block_addr = (uint32_t *) ALIGNED((uint32_t) frame_buffer.block_addr, 8);
  encIn.pOutBuf = frame_buffer.aligned_block_addr;
  encIn.busOutBuf = (uint32_t) encIn.pOutBuf;
  encIn.outBufSize = VENC_OUTPUT_BLOCK_SIZE;

  /* create stream */
  ret = H264EncStrmStart(encoder, &encIn, &encOut);
  if (ret != H264ENC_OK)
  {
    return -1;
  }
  frame_buffer.size = encOut.streamSize;
  if(tx_queue_send(&enc_frame_queue, (void *) &frame_buffer, TX_NO_WAIT) != TX_SUCCESS)
  {
    tx_block_release(frame_buffer.block_addr);
  }
  encIn.codingType = H264ENC_INTRA_FRAME;
  return 0;
}

__weak void timeMonitor(void) {};
uint32_t nb_encoded_frame = 0;

extern venc_h264_cfg_t hVencH264Instance;

/**
 * @brief Marks a frame with a square overlay for debugging or visualization purposes.
 *
 * This function draws a square of size SQUARE_SIZE on the provided frame buffer.
 * The frame is assumed to use 16 bits per pixel (BPP = 2).
 *
 * @param frame Pointer to the frame buffer to be marked.
 *
 * @warning PLS DO NOT USE THIS FCT in HwHandshake mode 
  *          as the frame buffer is not accessible.
 */
static void mark_frame(void * frame)
{
#if USE_MARK_FRAME
#define SQUARE_SIZE 50 
#define BPP         2 // Assume 16bpp

  static uint32_t  x0 = 0, y0 = 0;

  size_t offset = (hVencH264Instance.cfgH264Main.width * y0 + x0) * BPP;
  void * draw_ptr = (uint8_t *)frame + offset;
  
  for (uint8_t i = 0; i< SQUARE_SIZE; i++)
  {
    memset(draw_ptr, 0xff, SQUARE_SIZE*BPP); // Assume 16bpp
    offset =  hVencH264Instance.cfgH264Main.width  * BPP;
    draw_ptr = (uint8_t* )draw_ptr + offset;
   
  }
  
  x0 += SQUARE_SIZE;
  
  if (x0 + SQUARE_SIZE> hVencH264Instance.cfgH264Main.width)
  {
    x0= 0;
    y0 += SQUARE_SIZE;
  }
  
  if (y0 + SQUARE_SIZE > hVencH264Instance.cfgH264Main.height)
  {
    y0 = 0;
  }
#endif
}

/**
 * @brief  Encode a single frame: set input buffers, allocate output block,
 *         call encoder API and handle results (queue output or reset on error).
 * @retval int 0 on success, -1 on failure
 */
static int encode_frame(void)
{
  venc_output_frame_t frame_buffer = {0};
  int ret = H264ENC_FRAME_READY;
  if (!(frame_nb % 30))
  {
    /* if frame is the first : set as intra coded */
    encIn.codingType = H264ENC_INTRA_FRAME;
  }
  else
  {
    /* if there was a frame previously, set as predicted */
    encIn.timeIncrement = 1;
    encIn.codingType = H264ENC_PREDICTED_FRAME;
  }
  encIn.ipf = H264ENC_REFERENCE_AND_REFRESH;
  encIn.ltrf = H264ENC_REFERENCE;
  
  /* set input buffers to structures */
  encIn.busLuma = dcmipp_address.YAddress;
  
  /* used for multiplanar formats*/
  encIn.busChromaU = dcmipp_address.UAddress;
  encIn.busChromaV = dcmipp_address.VAddress;        

  mark_frame((void*)encIn.busLuma);
  
  /* allocate and set output buffer */
  if(tx_block_allocate(&venc_block_pool, (void **) &frame_buffer.block_addr, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    printf("VENC : failed to allocate output buffer\n");
    return -1;
  }
  frame_buffer.aligned_block_addr = (uint32_t *) ALIGNED((uint32_t) frame_buffer.block_addr, 8);
  encIn.pOutBuf = frame_buffer.aligned_block_addr;
  encIn.busOutBuf = (uint32_t) encIn.pOutBuf;
  encIn.outBufSize = VENC_OUTPUT_BLOCK_SIZE - 8;
  ret = H264EncStrmEncode(encoder, &encIn, &encOut, NULL, NULL, NULL);
  
  //timeMonitor();
 
  switch (ret)
  {
  case H264ENC_FRAME_READY:
    /*save stream */
    if(encOut.streamSize == 0)
    {
      encIn.codingType = H264ENC_INTRA_FRAME;
      tx_block_release(frame_buffer.block_addr);
      return -1;
    }
    frame_buffer.size = encOut.streamSize;
    if(tx_queue_send(&enc_frame_queue, (void *) &frame_buffer, TX_NO_WAIT) != TX_SUCCESS)
    {
      tx_block_release(frame_buffer.block_addr);
    }
    encIn.codingType = H264ENC_PREDICTED_FRAME;
    nb_encoded_frame++;
    break;
  case H264ENC_FUSE_ERROR:
    printf("DCMIPP and VENC desync (frame#%ld), restart the video\n", frame_nb);
    tx_block_release(frame_buffer.block_addr);
    encoder_reset();
    break;
  default:
    printf("error encoding frame %d\n", ret);
    tx_block_release(frame_buffer.block_addr);
    encIn.codingType = H264ENC_INTRA_FRAME;
    return -1;
    break;
  }
  frame_nb++;
  return 0;
}


/**
 * @brief  End encoding session: stop camera, finalize encoder stream and
 *         release/flush resources.
 * @retval int 0 on success, -1 on failure
 */
static int encoder_end(void){
  BSP_CAMERA_Stop(0);
  int ret = H264EncStrmEnd(encoder, &encIn, &encOut);
  if (ret != H264ENC_OK)
  {
    return -1;
  }
  tx_queue_flush(&enc_frame_queue);
  tx_block_pool_delete(&venc_block_pool);
  return 0;
}


/**
 * @brief This function configures the DCMIPP clock source and dividers to achieve
 *        the desired clock frequencies.
 * @param [hdcmipp] Pointer to a DCMIPP_HandleTypeDef structure that contains
 *                  the configuration information for the DCMIPP peripheral.
 * @return HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure.
 */
HAL_StatusTypeDef MX_DCMIPP_ClockConfig(DCMIPP_HandleTypeDef *hdcmipp)
{
 RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* Configure DCMIPP ck_ker_dcmipp to ic17 with PLL2 (1000MHz) / 3 = 333MHz */
  PeriphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_DCMIPP;
  PeriphClkInit.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
  PeriphClkInit.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  PeriphClkInit.ICSelection[RCC_IC17].ClockDivider = 4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    return HAL_ERROR;
  }
  LL_RCC_IC18_SetSource(LL_RCC_ICCLKSOURCE_PLL1);
  LL_RCC_IC18_SetDivider(60);   /* 800/40=20Mhz */
  LL_RCC_IC18_Enable();

  return HAL_OK;
}

/**
 * @brief Configures the DCMIPP downsize parameters to scale the camera input to the desired capture size.
 *
 * This function calculates and sets the horizontal and vertical downsize ratios and factors
 * for the DCMIPP peripheral, enabling the hardware to scale the camera input resolution
 * (camWidth x camHeight) down to the target capture resolution (captureWidth x captureHeight).
 *
 * @param hdcmipp        Pointer to the DCMIPP handle structure.
 * @param pipe           DCMIPP pipe number to configure.
 * @param camWidth       Width of the camera sensor input.
 * @param camHeight      Height of the camera sensor input.
 * @param captureWidth   Desired output width after downscaling.
 * @param captureHeight  Desired output height after downscaling.
 * @return HAL_OK on success, HAL_ERROR on failure.
 *
 * @note The function enables the downsize feature on the specified DCMIPP pipe.
 */
static HAL_StatusTypeDef  dcmipp_downsize(DCMIPP_HandleTypeDef *hdcmipp,int32_t pipe, int32_t camWidth,int32_t camHeight,int32_t captureWidth,int32_t captureHeight)
{
  /* Calcultation explained in RM0486v2 table 354*/
  
  DCMIPP_DownsizeTypeDef DonwsizeConf ={0};
  /* Configure the downsize */
  DonwsizeConf.HRatio      = (uint32_t)((((float)(camWidth)) / ((float)(captureWidth))) * 8192.F); 
  DonwsizeConf.VRatio      = (uint32_t)((((float)(camHeight )) / ((float)(captureHeight ))) * 8192.F); 
  DonwsizeConf.HSize       = captureWidth;
  DonwsizeConf.VSize       = captureHeight;

  DonwsizeConf.HDivFactor  = (uint32_t)floor((1024 * 8192 -1) / DonwsizeConf.HRatio);
  DonwsizeConf.VDivFactor  = (uint32_t)floor((1024 * 8192 -1) / DonwsizeConf.VRatio);

  if(HAL_DCMIPP_PIPE_SetDownsizeConfig(hdcmipp, pipe, &DonwsizeConf) != HAL_OK) return HAL_ERROR;
  if(HAL_DCMIPP_PIPE_EnableDownsize(hdcmipp, pipe)!= HAL_OK) return HAL_ERROR;
  return HAL_OK;
}

/**
 * @brief  Enables the hardware handshake for the DCMIPP peripheral.
 * @param  hdcmipp: Pointer to a DCMIPP_HandleTypeDef structure that contains
 *         the configuration information for the DCMIPP peripheral.
 * @retval HAL status
 */
static HAL_StatusTypeDef  dcmipp_enable_HW_Handshake(DCMIPP_HandleTypeDef *hdcmipp)
{
  uint32_t warpLines, irqLines;
    
  if (GetDCMIPPLinesConfig(&warpLines,&irqLines) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  if (HAL_DCMIPP_PIPE_SetLineWrappingConfig(hdcmipp, DCMIPP_PIPE1, warpLines) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  if (HAL_DCMIPP_PIPE_EnableLineWrapping(hdcmipp, DCMIPP_PIPE1) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  /* Enable line event with no interrupt to use the hardware trigger to the video encoder  */
  MODIFY_REG(DCMIPP->P1PPCR, DCMIPP_P1PPCR_LINEMULT_Msk,irqLines);
  return HAL_OK;
}

/**
 * @brief  Initializes the DCMIPP peripheral.
 * @param  hdcmipp: Pointer to a DCMIPP_HandleTypeDef structure that contains
 *         the configuration information for the DCMIPP peripheral.
 * @retval HAL status:
 *           - HAL_OK: Initialization successful
 *           - HAL_ERROR: Initialization error
 *           - HAL_BUSY: Peripheral busy
 *           - HAL_TIMEOUT: Timeout occurred
 *
 * This function configures and initializes the DCMIPP peripheral according to
 * the specified parameters in the DCMIPP handle. It must be called before
 * using the DCMIPP peripheral.
 */
HAL_StatusTypeDef MX_DCMIPP_Init(DCMIPP_HandleTypeDef *hdcmipp)
{
  DCMIPP_PipeConfTypeDef pPipeConf = {0};
  DCMIPP_CSI_PIPE_ConfTypeDef pCSIPipeConf = {0};
  DCMIPP_CSI_ConfTypeDef csiconf = {0};
  HAL_StatusTypeDef ret = HAL_OK;

  if (HAL_DCMIPP_Init(hdcmipp) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Configure the CSI */
  csiconf.DataLaneMapping = DCMIPP_CSI_PHYSICAL_DATA_LANES;
  csiconf.NumberOfLanes   = DCMIPP_CSI_TWO_DATA_LANES;
  csiconf.PHYBitrate      = DCMIPP_CSI_PHY_BT_1600;
  HAL_DCMIPP_CSI_SetConfig(hdcmipp, &csiconf);

  /* Configure the Virtual Channel 0 */
  /* Set Virtual Channel config */
  HAL_DCMIPP_CSI_SetVCConfig(hdcmipp, DCMIPP_VIRTUAL_CHANNEL0, DCMIPP_CSI_DT_BPP10);


  /* Configure the serial Pipe */
  pCSIPipeConf.DataTypeMode = DCMIPP_DTMODE_DTIDA;
  pCSIPipeConf.DataTypeIDA  = DCMIPP_DT_RAW10;
  pCSIPipeConf.DataTypeIDB  = DCMIPP_DT_RAW10; /* Don't Care */


  if (HAL_DCMIPP_CSI_PIPE_SetConfig(hdcmipp, DCMIPP_PIPE1, &pCSIPipeConf) != HAL_OK)
  {
    return HAL_ERROR;
  }

  pPipeConf.FrameRate  = DCMIPP_FRAME_RATE_ALL;  /* Sensor framerate is set with IMX335_SetFramerate(..FRAMERATE);*/
  pPipeConf.PixelPackerFormat = hDcmippH264Instance.format;

  /* Set Pitch for Main and Ancillary Pipes */
  pPipeConf.PixelPipePitch  =hDcmippH264Instance.pitch; /* Number of bytes */

  /* Configure Pipe */
  if (HAL_DCMIPP_PIPE_SetConfig(hdcmipp, DCMIPP_PIPE1, &pPipeConf) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DCMIPP_PIPE_EnableRedBlueSwap(hdcmipp, DCMIPP_PIPE1) != HAL_OK)
  {
    return HAL_ERROR;
  }
  #define N10(val) (((val) ^ 0x7FF) + 1)
  DCMIPP_ColorConversionConfTypeDef color_conf = {
    .ClampOutputSamples = ENABLE,
    .OutputSamplesType = DCMIPP_CLAMP_YUV,
    .RR = N10(26), .RG = N10(87), .RB = 112, .RA = 128,
    .GR = 47, .GG = 157, .GB = 16, .GA = 16,
    .BR = 112, .BG = N10(102), .BB = N10(10), .BA = 128,
  };
  if (HAL_DCMIPP_PIPE_SetYUVConversionConfig(hdcmipp, DCMIPP_PIPE1, &color_conf) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DCMIPP_PIPE_EnableYUVConversion(hdcmipp, DCMIPP_PIPE1) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Configure DCMIPP output size*/
  ret= dcmipp_downsize(hdcmipp,DCMIPP_PIPE1, 
                  hCamH264Instance.width, 
                  hCamH264Instance.height,
                  hVencH264Instance.cfgH264Main.width,
                  hVencH264Instance.cfgH264Main.height);


  
  if (ret == HAL_OK)
  {
  /* Specific Hardware Handshake if needed*/
    if (IsHwHanshakeMode())
  {
    ret = dcmipp_enable_HW_Handshake(hdcmipp);
  }
    else  /* Frame Mode : avoid spurious interrupts */
    { 
      /* Software workaround for Linemult triggering VENC interrupt. Minimize occurrences as much as possible. */
      MODIFY_REG(DCMIPP->P1PPCR, DCMIPP_P1PPCR_LINEMULT_Msk,DCMIPP_MULTILINE_128_LINES);

      /* Disable Line events interrupts*/
      ret = HAL_DCMIPP_PIPE_DisableLineEvent(hdcmipp, DCMIPP_PIPE1);
    }
  }
  return ret;
}

/**
 * @brief  Callback function called when a camera frame event occurs.
 * @param  instance Camera instance number that triggered the event.
 * @note   This function should be implemented to handle actions required
 *         when a new camera frame is available (e.g., processing or displaying the frame).
 */
void BSP_CAMERA_FrameEventCallback(uint32_t instance)
{
  /* signal new frame*/
  nbLineEvent = 0;
  frame_received++;
  tx_event_flags_set(&venc_app_flags, FRAME_RECEIVED_FLAG, TX_OR);
}

/**
 * @brief  Callback function called when a camera line event occurs.
 * @param  instance Camera instance that triggered the event.
 * @retval None
 *
 * This function is typically called by the BSP (Board Support Package) layer
 * when a line event is detected by the camera hardware. The user can implement
 * this callback to handle line-based processing or synchronization.
 */
void BSP_CAMERA_LineEventCallback(uint32_t instance)
{
  /* signal new frame*/
 nbLineEvent++;
}

/**
 * @brief Starts the video encoding process.
 *
 * This function initializes and starts the video encoding operation.
 */
void VENC_APP_EncodingStart(void)
{
  /* initialize encoder software for camera feed encoding */
  encoder_start();
  tx_event_flags_set(&venc_app_flags, VIDEO_START_FLAG, TX_OR);
}

/**
 * @brief Retrieves encoded video data for transmission or processing.
 *
 * This function provides access to the next available chunk of encoded video data.
 * The function sets the pointer to the data buffer and its size.
 *
 * @param[out] data Pointer to a variable that will receive the address of the data buffer.
 * @param[out] size Pointer to a variable that will receive the size (in bytes) of the data buffer.
 *
 * @return UINT Status code of the operation (e.g., 0 for success, error code otherwise).
 */
UINT VENC_APP_GetData(UCHAR **data, ULONG *size)
{
  static uint32_t * curr_block = NULL;
  if(curr_block)
  {
    tx_block_release(curr_block);
    curr_block = NULL;
  }
  venc_output_frame_t frame_block;
  if(tx_queue_receive(&enc_frame_queue, (void *) &frame_block, TX_NO_WAIT) != TX_SUCCESS)
  {
    *data = NULL;
    *size = 0;
    return(-1);
  }
  *data = (UCHAR *) frame_block.aligned_block_addr;
  *size = frame_block.size + 15;
  curr_block = frame_block.block_addr;
  return(0);
}

/**
 * @brief Stops the video encoding process.
 *
 * @return  0 
 */
UINT VENC_APP_EncodingStop(void)
{
  ULONG flags;
  tx_event_flags_get(&venc_app_flags, VIDEO_START_FLAG, TX_AND_CLEAR, &flags, TX_WAIT_FOREVER);
  (void) encoder_end();
  return(0);
}

/**
 * @brief  Initializes the audio subsystem.
 * 
 * This is a weak implementation of the audio initialization function.
 * Users can override this function to provide custom audio initialization
 * routines as required by their application.
 *
 * @note  This function should be implemented by the user if audio features
 *        are needed. The default weak implementation does nothing.
 */
__weak void audio_init(void)
{
}

/**
 * @brief  Starts the audio processing or streaming.
 * @note   This is a weak implementation and can be overridden by user-defined function.
 * @retval None
 */
__weak void audio_start(void)
{
}


/**
 * @brief  Starts the audio encoding process.
 * 
 * @retval 0 
 */
UINT AUDIO_APP_EncodingStart(void)
{
  audio_init(); 
  audio_start();
  return 0;
}


/**
 * @brief  Retrieves audio data for processing or transmission.
 * @note   This is a weak implementation and can be overridden by user-defined functions.
 * @param  data: Pointer to a pointer that will be set to the address of the audio data buffer.
 * @param  size: Pointer to a variable that will be set to the size (in bytes) of the audio data buffer.
 * @retval uint32_t: Status code (e.g., 0 for success, non-zero for error).
 */
__weak uint32_t AUDIO_APP_GetData(uint8_t **data, size_t *size)
{
  *size = 0;
  return 0;
}


