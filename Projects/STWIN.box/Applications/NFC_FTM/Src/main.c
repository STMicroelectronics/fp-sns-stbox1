/**
  ******************************************************************************
  * @file    NFC_FTM\Src\main.c
  * @author  System Research & Applications Team - Catania Lab.
  * @version V1.6.0
  * @date    20-Oct-2023
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/**
 *
 * @page NFC_FTM NFC Fast Transfer Memory protocol 
 *
 * @image html st_logo.png
 *
 * <b>Introduction</b>
 *
 * This firmware package includes Components Device Drivers, Board Support Package
 * and example application for the following STMicroelectronics elements:
 * - STEVAL-STWINBX1 (STWIN.box) evaluation board that contains the following components:
 *   - MEMS sensor devices: IIS2DLPC, IIS2MDC, IIS3DWB, ISM330DHCX, IIS2ICLX, ILPS22QS, STTS22H
 *   - analog (IMP23ABSU) and digital (IMP34DT05) microphones
 *   - dynamic NFC tag: ST25DV64
 *   - BlueNRG-2 Bluetooth Low Energy System On Chip

 * <b>Example Application</b>

 * The Example application provides one example on how to use the Fast Memory Transfer protocol for making a firmware update using the NFC and using 
 * the STMicroelectronics “ST25 NFC Tap” available for Android and iOS:
 *   - https://play.google.com/store/apps/details?id=com.st.st25nfc
 *   - https://apps.apple.com/be/app/nfc-tap/id1278913597
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "STBOX1_config.h"
#include "STWIN.box.h"
#include "STWIN.box_nfctag.h"
#include "st25ftm_config.h"
#include "st25ftm_process.h"

/* Exported variables --------------------------------------------------------*/

TIM_HandleTypeDef TimCCHandle;
int32_t CurrentActiveBank = 0;

/* Imported variables --------------------------------------------------------*/


/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t user_button_pressed = 0;
/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_ICACHE_Init(void);
static void User_Init(void);
static void PrintInfo(void);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and tcd c:
he Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_ICACHE_Init();
  
  User_Init();
  
  STBOX1_PRINTF("\033[2J"); /* serial console clear screen */
  STBOX1_PRINTF("\033[H");  /* serial console cursor to home */
  PrintInfo();
  
  /* Init ST25DV driver */
  while( BSP_NFCTAG_Init(BSP_NFCTAG_INSTANCE) != NFCTAG_OK );
  
  /* Setting the New Password for I2C protection */
  if(BSP_NFCTAG_ChangeI2CPassword(STBOX1_MSB_PASSWORD,STBOX1_LSB_PASSWORD)!=NFCTAG_OK ) {
    STBOX1_PRINTF("Error NFC Changing the I2C password\r\n");
    Error_Handler(STBOX1_ERROR_NFC,__FILE__,__LINE__);
  } else {
    STBOX1_PRINTF("NFC Changed the I2C password\r\n");
  }
  
  /* Enable the NFC GPIO Interrupt */
  if(BSP_NFCTAG_GPIO_Init()!=BSP_ERROR_NONE) {
    STBOX1_PRINTF("Error NFC Initializing GPIO\r\n");
    Error_Handler(STBOX1_ERROR_NFC,__FILE__,__LINE__);
  } else {
    STBOX1_PRINTF("NFC NFC GPIO Initialized\r\n");
  }
  
  STBOX1_PRINTF("Press User Button for Enabling/Disabling FTM\r\n");
  
  /* Infinite loop */
  while (1) {
    static uint32_t MailBoxEnabled=0;
    
    /* FTM Work Fuction*/
    if(MailBoxEnabled==1) {
      FTMManagement();
    }
    
    /* Handle the user button */
    if(user_button_pressed) {
      user_button_pressed=0;
      if(MailBoxEnabled) {
        MailBoxEnabled=0;
        FTMManagementDeInit();
      } else {
        MailBoxEnabled =1;
        FTMManagementInit();
      }
    }

    /* Wait next event */
    __WFI();
  }
}

#if defined (__IAR_SYSTEMS_ICC__)
extern UART_HandleTypeDef hcom_uart[];
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  (void)HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)buffer, 1, COM_POLL_TIMEOUT);
  return size;
}
#endif

/**
* @brief  Initialize User process
*
* @param  None
* @retval None
*/
static void User_Init(void)
{
  /* Enable Button in Interrupt mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /* Init the Led */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_ORANGE);
  
  /* Check if we are running from Bank1 or Bank2 */
  {
    FLASH_OBProgramInitTypeDef    OBInit;
    /* Allow Access to Flash control registers and user Flash */
    HAL_FLASH_Unlock();
    /* Allow Access to option bytes sector */ 
    HAL_FLASH_OB_Unlock();
    /* Get the Dual boot configuration status */
    HAL_FLASHEx_OBGetConfig(&OBInit);
    if (((OBInit.USERConfig) & (OB_SWAP_BANK_ENABLE)) == OB_SWAP_BANK_ENABLE) {
      CurrentActiveBank= 2;
      MCR_HEART_BIT2();
    } else {
      CurrentActiveBank= 1;
      MCR_HEART_BIT();
    }
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
  }

  BSP_COM_Init(COM1);
}

/**
* @brief  BSP Push Button callback
*
* @param  Button Specifies the pin connected EXTI line
* @retval None
*/
void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Set the User Button flag */
  user_button_pressed = 1;
}

/**
 * @brief  BSP NFCTAG GPIO callback
 * @param  Node
 * @retval None.
 */
void BSP_NFCTAG_GPIO_Callback(void)
{
  /* Set the GPIO flag */
  GPO_Activated=1;
}

/**
* @brief  Print Bunner
* @param  None
* @retval None
*/
static void PrintInfo(void)
{
  STBOX1_PRINTF("\r\nSTMicroelectronics %s:\r\n"
            "\tVersion %d.%d.%d\r\n"
              "\tSTM32U585AI-STWIN.box board"
                "\r\n",
                STBOX1_PACKAGENAME,
                STBOX1_VERSION_MAJOR,STBOX1_VERSION_MINOR,STBOX1_VERSION_PATCH);
  STBOX1_PRINTF("\t(HAL %ld.%ld.%ld_%ld)\r\n"
            "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
              " (IAR)\r\n",
#elif defined (__ARMCC_VERSION)
              " (KEIL)\r\n",
#elif defined (__GNUC__)
              " (STM32CubeIDE)\r\n",
#endif
              HAL_GetHalVersion() >>24,
              (HAL_GetHalVersion() >>16)&0xFF,
              (HAL_GetHalVersion() >> 8)&0xFF,
              HAL_GetHalVersion()      &0xFF,
              __DATE__,__TIME__);
  STBOX1_PRINTF("Current Bank =%ld\r\n",CurrentActiveBank);
}

#if defined (__ARMCC_VERSION)
extern UART_HandleTypeDef hcom_uart[];
/**
 * @brief fputc call for standard output implementation
 * @param ch Character to print
 * @param f File pointer
 * @retval Character printed
 */
int fputc(int ch, FILE *f)
{
  (void) HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*) &ch, 1, COM_POLL_TIMEOUT);
  return ch;
}

/** @brief fgetc call for standard input implementation
 * @param f File pointer
 * @retval Character acquired from standard input
 */
int fgetc(FILE *f)
{
  int ch;
	(void)HAL_UART_Receive(&hcom_uart[COM1], (uint8_t*) &ch, 1, COM_POLL_TIMEOUT);
	return ch;
}
#endif

/**
  * @brief System Clock Configuration 128MHz
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_0;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV4;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }
    
  __HAL_RCC_PWR_CLK_DISABLE();
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{
  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK){
    Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }
  if (HAL_ICACHE_Enable() != HAL_OK){
    Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param int32_t ErrorCode Error Code
  * @retval None
  */
void Error_Handler(int32_t ErrorCode,char *File,int32_t Line)
{
  /* User can add his own implementation to report the HAL error return state */
  BSP_LED_Off(LED_ORANGE);
  STBOX1_PRINTF("Error at %d at %s\r\n",Line,File);
  while (1){
    int count;
    for(count=0;count<ErrorCode;count++) {
      BSP_LED_On(LED_ORANGE);
      HAL_Delay(500);
      BSP_LED_Off(LED_ORANGE);
      HAL_Delay(2000);
    }
    BSP_LED_On(LED_GREEN);
    HAL_Delay(2000);
    BSP_LED_Off(LED_GREEN);
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

