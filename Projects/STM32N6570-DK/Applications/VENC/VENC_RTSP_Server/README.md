
##  <b>VENC_RTSP_Server Application Description</b>

This application provides an example of the H264 video encoder streamed through Azure RTOS NetX/NetXDuo on STM32N6570-DK board.
It allows easy testing of the following use cases:
+ h264 encoding in 1080p, 720p, or 480p in Frame mode or Hardware Handshake mode (aka "Slice mode" aka "Streaming mode")
+ h264 + Audio uncompressed (PCM)

The application aims to stream an encoded H264 video (+ audio) format. 

Then,the frames are sent through Ethernet peripheral using the RTP protocol (Real-time Transport Protocol) to a remote client (for example VLC media player or ffmpeg). 

The RTSP protocol (Real Time Streaming Protocol) is used to control
media sessions (SETUP, PLAY, TEARDOWN).


### Thread Overview

The main entry function tx_application_define() is called by ThreadX during kernel start.
The following threads are created.

**User Threads:**

- **Monitor App Thread** (`monitor_thread_func`, priority 63):  
  Monitors system performance metrics such as CPU load and bitrates.

- **VENC App Thread** (`venc_thread_func`, priority 12):  
  Handles video encoding operations, manages the video capture pipeline, and streams encoded video frames over the network.

**NetXDuo Threads:**

- **App Main Thread** (`App_Main_Thread_Entry`, priority 10):  
  Manages the main application logic, coordinating initialization, control flow, and high-level events for the RTSP server application.

- **App Link Thread** (`App_Link_Thread_Entry`, priority 11):  
  Handles the initialization and management of the application’s network link status and related events.

- **NetX DHCP Client** (`_nx_dhcp_thread_entry`, priority 3):  
  Manages the DHCP client operations, including requesting and renewing the IP address for the network interface.

- **RTSP Server** (`_nx_rtsp_server_thread_entry`, priority 3):  
  Manages the RTSP server operations, handling client connections, session control, and streaming setup for real-time audio/video transmission.

- **Test Thread** (`app_server_entry`, priority 4):  
  Manages the RTSP server thread, handling client connections, session control, and coordinating video/audio streaming operations.

- **Main IP Instance** (`_nx_ip_thread_entry`, priority 1):  
  Manages the IP stack operations, including packet processing, routing, and network interface management for the application.

####  <b>Expected success behavior</b>

+ The board IP address is printed on the HyperTerminal@115200 bauds

#####  <b>Playback using ffplay</b>
Playback with low latency can be done using ffplay ;
```sh
ffplay -flags low_delay rtsp://[IP]
```
 Note : This application was tested with ffplay version 6.1.1-3ubuntu5

#####  <b>Playback using VLC</b>
Enter rtsp://[IP]:554 in the Media/Open Network Stream window.
 
 + Streaming video is well displayed on the VLC media player after entering the URL: rtsp://[IP]:554  in the Media/Open Network Stream/Open Media window
  (example rtsp://192.168.0.27:554)
 + It is recommended that the caching value is set to 500ms on the VLC to keep the stream running smoothly
  (Media/Open Network Stream/Open Media window => Show more options), then press Play on VLC Play button.
 + The response messages sent by the server are printed on the HyperTerminal.

 Note : This application was tested with VLC version 3.0.19
 Limitation: When using PCM, VLC audio rendering is scattered. This issue is pending resolution.

#### <b>Expected error behavior</b>

+ The red LED toggles every 1000ms to indicate that an error has occurred.
+ In case the message exchange is not completed the Hyperterminal does not print the received messages.
+ ""Critical error has occurred" message is printed on the HyperTerminal.

#### <b>Assumptions if any</b>

- The application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the RTL used to test the application.
- The application is configuring the Ethernet IP with a static predefined <i>MAC Address</i>. Ensure to change it if multiple boards are connected on the same RTL to avoid any potential network traffic issues.
- The <i>MAC Address</i> is defined in the `main.c`

```
void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH1;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE0;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x10;
  MACAddr[5] = 0x00;
```
#### <b>Known limitations</b>

The RTSP server is minimalist and does not support client multiple connections/disconnections.

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be considered when using delays or timeouts at application. It can be reconfigured by updating the "TX_TIMER_TICKS_PER_SECOND" define in the "tx_user.h" file. The update should be reflected in "tx_initialize_low_level.S" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function using the first_unused_memory argument.
   This requires changes in the linker files to expose this memory location.
    + For EWARM add the following section into the .icf file:
     ```
	 place in RAM_region    { last section FREE_MEM };


#### <b>NetX Duo usage hints</b>

- Depending on the application scenario, the total TX and RX descriptors may need to be increased by updating respectively  the "ETH_TX_DESC_CNT" and "ETH_RX_DESC_CNT" in the "stm32n6xx_hal_conf.h", to ensure the application correct behaviour, but this will cost extra memory to allocate.
- The NetXDuo application needs to allocate a specific pool in a uncached section.
Below is an example of the declaration of the nx pool.

```
  #if (USE_STATIC_ALLOCATION == 1)
  static UCHAR nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE] __ALIGN_END __NON_CACHEABLE;
  static TX_BYTE_POOL nx_app_byte_pool;
  #endif
```

with 
```
#define __NON_CACHEABLE __attribute__((section(".noncacheable")))
```

The definition of the uncached section must be consistent between the link file and the MPU configuration.


For more details about the MPU configuration please refer to the [AN4838](https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf)

### <b>Keywords</b>

RTOS, Network, ThreadX, NetXDuo, RTP, RTSP, TCP, UDP, UART

### <b>Hardware and Software environment</b>

  - This application runs on STM32N657xx devices.
  - This application has been tested with STMicroelectronics STM32N6570-DK boards Revision MB1939-N6570-A03 and can be easily tailored to any other supported device and development board.

  - This application uses USART1 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None

  - **EWARM** : To monitor a variable in the live watch window, you must proceed as follow :
    - Start a debugging session.
    - Open the View > Images.
    - Double-click to deselect the second instance of project.out.

  - **MDK-ARM** : To monitor a variable in the live watch window, you must comment out SCB_EnableDCache() in main() function.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Set the boot mode in development mode (BOOT1 switch position is 1-3, BOOT0 switch position doesn't matter).
 - Open your preferred toolchain
 - Select first the FSBL workspace
 - Rebuild all files from sub-project FSBL (if no modification is done on FSBL project, this step can be done only once)
 - Select the Appli workspace
 - Rebuild all files from sub-project Appli
 - Resort to CubeProgrammer to add a header to the generated Appli binary Project.bin with the following command
   - *STM32_SigningTool_CLI.exe -bin Project.bin -nk -of 0x80000000 -t fsbl -o Project-trusted.bin -hv 2.3 -dump Project-trusted.bin*
   - The resulting binary is Project-trusted.bin.
 - Next, in resorting again to CubeProgrammer, load the Appli binary and its header (Project-trusted.bin) in DK board external Flash at address 0x7010'0000.
 - Load the FSBL binary in internal RAM using the IDE
 - Run the example

 Next, this program can be run in boot from flash mode. This is done by following the instructions below:

 - Resort to CubeProgrammer to add a header to the generated binary FSBL.bin with the following command
   - *STM32_SigningTool_CLI.exe -bin FSBL.bin -nk -of 0x80000000 -t fsbl -o FSBL-trusted.bin -hv 2.3 -dump FSBL-trusted.bin*
   - The resulting binary is FSBL-trusted.bin.
 - In resorting again to CubeProgrammer, load the FSBL binary and its header (FSBL-trusted.bin) in DK board external Flash at address 0x7000'0000.
 - Set the boot mode in boot from external Flash (BOOT0 switch position is 1-2 and BOOT1 switch position is 1-2).
 - Press the reset button. The code then executes in boot from external Flash mode.

Note :
-  when using CubeIDE, make sure to use the Debug configuration for development mode and the Release configuration for boot from flash.
- 2 scripts have been added under Projects/STM32N6570-DK/Applications/VENC/VENC_RTSP_Server_Ext/Tools. These are helper to flash IAR and CubeIDE configurations. They are provided as reference and need adaptation to each environment (path)