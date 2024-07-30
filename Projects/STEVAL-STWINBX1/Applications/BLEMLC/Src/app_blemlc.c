/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_blemlc.c
  * @author  System Research & Applications Team - Agrate/Catania Lab.
  * @brief   Main program body
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

/**
 *
 * @page BLEMLC BLE Firmware example for Machine Learning Core and Finite State Machine
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
 *
 * <b>Example Application</b>
 *
 * This example explains how to use Machine Learning Core and the Finite State Machine
 * This example is compatible with the Firmware Over the Air Update (FoTA)
 *
 *
 * This example must be used with the related ST BLE Sensor Android/iOS application available on Play/itune store
 * (Version 4.18.0 or higher), in order to read the sent information by Bluetooth Low Energy protocol
 *
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "BLE_Function.h"
#include "app_blemlc.h"
#include "steval_stwinbx1.h"
#include "bluenrg_conf.h"
#include "OTA.h"
#include "BLE_Manager.h"
#include "STWIN.box_motion_sensors.h"
#include "STWIN.box_motion_sensors_ex.h"
#include "STBOX1_config.h"

/* Default program for Machine Learning Core */
#include "ism330dhcx_motion_intensity.h"

/* Default program for Finite State Machine */
#include "ism330dhcx_four_d.h"

/* Exported variables --------------------------------------------------------*/
uint16_t ConnectionHandle = 0;

int32_t CurrentActiveBank = 0;

/* Imported variables --------------------------------------------------------*/

/* Custom UCF files for MLC and FSM */
ucf_line_t *MLCCustomUCFFile = NULL;
uint32_t MLCCustomUCFFileLength=0;
ucf_line_t *FSMCustomUCFFile = NULL;
uint32_t FSMCustomUCFFileLength=0;

/* Labels for Custom UCF Files form MLC and FSM */
char *MLCCustomLabels=NULL;
char *FSMCustomLabels=NULL;
uint32_t MLCCustomLabelsLength = 0;
uint32_t FSMCustomLabelsLength = 0;

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t UserButtonPressed = 0;
static volatile uint32_t BlinkLed        = 0;
static volatile uint32_t SendAccGyroMag  = 0;
static volatile uint32_t MEMSInterrupt   = 0;

/* Private function prototypes -----------------------------------------------*/
static void User_Init(void);
static void InitTimers(void);
static void PrintInfo(void);
static void MEMSCallback(void);
static void InitMems(void);
static void hexti8_callback(void);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval none
  */
void MX_BLEMLC_Init(void)
{
  /* Set a random seed */
  srand(HAL_GetTick());

  User_Init();

  STBOX1_PRINTF("\033[2J"); /* serial console clear screen */
  STBOX1_PRINTF("\033[H");  /* serial console cursor to home */
  PrintInfo();

  /* Init Magneto and Interrupt for Acc/Gyro */
  InitMems();

  /* Init BLE */
  STBOX1_PRINTF("\r\nInitializing Bluetooth\r\n");

  /* Characteristic allocation for FSM 16 Registers */
  CustomReadFiniteStateMachine = ReadFiniteStateMachine;

  /* Characteristic allocation for MLC 8 Registers */
  CustomReadMachineLearningCore = ReadMachineLearningCore;

  BluetoothInit();
  /* FOTA and Dual Banks Section */
  {
    uint16_t FwId1,FwId2;

    ReadFlashBanksFwId(&FwId1,&FwId2);
    if(FwId2!=OTA_OTA_FW_ID_NOT_VALID) {
      /* Enable the Banks Swap only if there is a valid fw on second bank */
      CustomExtConfigBanksSwapCommandCallback           = ExtConfigBanksSwapCommandCallback;
    }
  }

  /* Short delay before starting the user application process */
  HAL_Delay(500);
  STBOX1_PRINTF("BLE Stack Initialized & Device Configured\r\n");

}

/*
 * FP-SNS-STBOX1 background task
 */
void MX_BLEMLC_Process(void)
   {
    /* BLE Event */
    {
      hci_user_evt_proc();
    }

    /* Make the device discoverable */
    if(set_connectable)
    {
      uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TIM_CC_HANDLE);
      /* Start the TIM Base generation in interrupt mode */
      if(HAL_TIM_OC_Start_IT(&TIM_CC_HANDLE, TIM_CHANNEL_1) != HAL_OK){
        /* Starting Error */
        STBOX1_Error_Handler(STBOX1_ERROR_TIMER,__FILE__,__LINE__);
      }
      /* Set the Capture Compare Register value */
      __HAL_TIM_SET_COMPARE(&TIM_CC_HANDLE, TIM_CHANNEL_1, (uhCapture + STBOX1_UPDATE_LED_BATTERY));

      setConnectable();
      set_connectable = FALSE;
    }

    if (SendAccGyroMag){
      BLE_MANAGER_INERTIAL_Axes_t x_axes;
      BLE_MANAGER_INERTIAL_Axes_t g_axes;
      BLE_MANAGER_INERTIAL_Axes_t m_axes;
      SendAccGyroMag=0;

     BSP_MOTION_SENSOR_GetAxes(ISM330DHCX_0, MOTION_ACCELERO,(BSP_MOTION_SENSOR_Axes_t*)&x_axes);
     BSP_MOTION_SENSOR_GetAxes(ISM330DHCX_0, MOTION_GYRO,(BSP_MOTION_SENSOR_Axes_t*)&g_axes);
     BSP_MOTION_SENSOR_GetAxes(IIS2MDC_0, MOTION_MAGNETO,(BSP_MOTION_SENSOR_Axes_t*)&m_axes);
     BLE_AccGyroMagUpdate(&x_axes,&g_axes,&m_axes);
   }

    /* Reboot the Board */
    if(RebootBoard) {
      RebootBoard=0;
      HAL_NVIC_SystemReset();
    }

    /* Swap the Flash Banks */
    if(SwapBanks) {
      EnableDisableDualBoot();
      SwapBanks=0;
    }

    /* Handle the user button */
    if(UserButtonPressed) {
      UserButtonPressed=0;
      STBOX1_PRINTF("User Button pressed...\r\n");
    }

    /* Handle the MEMS interrupt */
    if(MEMSInterrupt) {
      MEMSInterrupt=0;
      MEMSCallback();
    }

    /* Blinking the Led */
    if(BlinkLed) {
      BlinkLed = 0;
      BSP_LED_Toggle(LED_GREEN);
    }

    /* Wait next event */
    __WFI();
  }

/**
  * @brief  Callback Interrupt 8
  * @param  None
  * @retval None
  */
static void hexti8_callback(void)
{
  MEMSInterrupt=1;
}

 /** @brief  Callback for ISM330DHCX interrupt
  * @param  None
  * @retval None
  */
static void MEMSCallback(void)
{
  ism330dhcx_all_sources_t      statusSources;
  ism330dhcx_mlc_status_mainpage_t statusPage;
  uint8_t MLCStatus;
  uint8_t FSMStatusA;
  uint8_t FSMStatusB;

  STBOX1_PRINTF("MEMSCallback\n\r");

  ism330dhcx_all_sources_get(ISM330DHCX_Contex, &statusSources);

  ism330dhcx_mlc_status_get(ISM330DHCX_Contex,&statusPage);

  MLCStatus = (statusPage.is_mlc1) | (statusPage.is_mlc2<<1) |
    (statusPage.is_mlc3<<2) | (statusPage.is_mlc4<<3) |
    (statusPage.is_mlc5<<4) | (statusPage.is_mlc6<<5) |
    (statusPage.is_mlc7<<6) | (statusPage.is_mlc8<<7);

  FSMStatusA = (statusSources.fsm_status_a.is_fsm1) | (statusSources.fsm_status_a.is_fsm2<<1) |
    (statusSources.fsm_status_a.is_fsm3<<2) | (statusSources.fsm_status_a.is_fsm4<<3) |
    (statusSources.fsm_status_a.is_fsm5<<4) | (statusSources.fsm_status_a.is_fsm6<<5) |
    (statusSources.fsm_status_a.is_fsm7<<6) | (statusSources.fsm_status_a.is_fsm8<<7);

  FSMStatusB = (statusSources.fsm_status_b.is_fsm9 ) | (statusSources.fsm_status_b.is_fsm10<<1) |
    (statusSources.fsm_status_b.is_fsm11<<2) | (statusSources.fsm_status_b.is_fsm12<<3) |
    (statusSources.fsm_status_b.is_fsm13<<4) | (statusSources.fsm_status_b.is_fsm14<<5) |
    (statusSources.fsm_status_b.is_fsm15<<6) | (statusSources.fsm_status_b.is_fsm16<<7);

  if(MLCStatus!=0){
    uint8_t mlc_out[8];
    ism330dhcx_mlc_out_get(ISM330DHCX_Contex, mlc_out);
    /* Check if we need to update the MLC BLE char */
    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_MLC)) {
      BLE_MachineLearningCoreUpdate(mlc_out,&MLCStatus);
    }
  } else if((FSMStatusA!=0) | (FSMStatusB!=0)){
    ism330dhcx_fsm_out_t fsm_out;
    ism330dhcx_fsm_out_get(ISM330DHCX_Contex, &fsm_out);

    if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_FSM)) {
      BLE_FiniteStateMachineUpdate((uint8_t *)&fsm_out,&FSMStatusA, &FSMStatusB);
    }
  }
}

/**
* @brief  Init Magneto and Interrupt for Acc/Gyro
* @param  None
* @retval None
*/
static void InitMems(void)
{
  /* Magneto */
  if(BSP_MOTION_SENSOR_Init(IIS2MDC_0, MOTION_MAGNETO)==BSP_ERROR_NONE) {
    if(BSP_MOTION_SENSOR_SetOutputDataRate(IIS2MDC_0, MOTION_MAGNETO, IIS2MDC_MAG_ODR)==BSP_ERROR_NONE) {
      if(BSP_MOTION_SENSOR_SetFullScale(IIS2MDC_0, MOTION_MAGNETO, IIS2MDC_MAG_FS)==BSP_ERROR_NONE) {
        STBOX1_PRINTF("IIS2MDC OK\r\n");
      } else {
        STBOX1_PRINTF("Error: IIS2MDC KO\r\n");
      }
    }else {
      STBOX1_PRINTF("Error: IIS2MDC KO\r\n");
    }
  } else {
    STBOX1_PRINTF("Error: IIS2MDC KO\r\n");
  }

  {
    /* Enable interrupts from INT1 ISM330DHCX  */
    GPIO_InitTypeDef GPIO_InitStruct;

    BSP_ISM330DHCX_INT1_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin = BSP_ISM330DHCX_INT1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BSP_ISM330DHCX_INT1_PORT, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_EXTI_GetHandle(&H_EXTI_INT1_ISM330DHCX, ISM330DHCX_INT1_EXTI_LINE);
    HAL_EXTI_RegisterCallback(&H_EXTI_INT1_ISM330DHCX, HAL_EXTI_COMMON_CB_ID, hexti8_callback);
    HAL_NVIC_SetPriority(BSP_ISM330DHCX_INT1_EXTI_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(BSP_ISM330DHCX_INT1_EXTI_IRQn);

    STBOX1_PRINTF("Enabled ISM330DHCX INT1 Detection \n\r");
  }
}

/** @brief Initialize the ISM330DHCX MEMS Sensor
 * @param None
 * @retval None
 */
void InitAcc(void)
{
  /* Acc/Gyro */
  if(BSP_MOTION_SENSOR_Init(ISM330DHCX_0, MOTION_ACCELERO | MOTION_GYRO)==BSP_ERROR_NONE) {
    if(BSP_MOTION_SENSOR_SetOutputDataRate(ISM330DHCX_0, MOTION_ACCELERO, ISM330DHCX_ACC_ODR)==BSP_ERROR_NONE) {
      if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_ACCELERO, ISM330DHCX_ACC_FS)==BSP_ERROR_NONE) {
        if(BSP_MOTION_SENSOR_SetOutputDataRate(ISM330DHCX_0, MOTION_GYRO, ISM330DHCX_GYRO_ODR)==BSP_ERROR_NONE) {
          if(BSP_MOTION_SENSOR_SetFullScale(ISM330DHCX_0, MOTION_GYRO, ISM330DHCX_GYRO_FS)==BSP_ERROR_NONE) {
            STBOX1_PRINTF("ISM330DHCX OK\r\n");
          } else {
            STBOX1_PRINTF("Error: ISM330DHCX KO\r\n");
          }
        } else {
          STBOX1_PRINTF("Error: ISM330DHCX KO\r\n");
        }
      } else {
        STBOX1_PRINTF("Error: ISM330DHCX KO\r\n");
      }
    } else {
      STBOX1_PRINTF("Error: ISM330DHCX KO\r\n");
    }
  } else {
    STBOX1_PRINTF("Error: ISM330DHCX KO\r\n");
  }
}

/** @brief DeInitialize the ISM330DHCX MEMS Sensor
 * @param None
 * @retval None
 */
void DeInit_Acc(void)
{
  /* DeInit Accelero */
  if (BSP_MOTION_SENSOR_DeInit(ISM330DHCX_0) == BSP_ERROR_NONE) {
    STBOX1_PRINTF("OK Deinit ISM330DHCX_0  Sensor\n\r");
  } else {
    STBOX1_PRINTF("Error Deinit ISM330DHCX_0 Sensor\n\r");
    STBOX1_Error_Handler(STBOX1_ERROR_SENSOR,__FILE__,__LINE__);
  }
}

/** @brief Initialize the ISM330DHCX MEMS Sensor for MLC
 * @param uint32_t UseCustomIfAvailableflag for Using or not the Custom UCF file
 * @retval None
 */
void InitAcc_MLC(uint32_t UseCustomIfAvailable)
{
  ucf_line_t *ProgramPointer;
  int32_t LineCounter;
  int32_t TotalNumberOfLine;
  int32_t RetValue;

  /* Init Accelero */
  if (BSP_MOTION_SENSOR_Init(ISM330DHCX_0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    STBOX1_PRINTF("OK Init Accelero  Sensor\n\r");
  } else {
    STBOX1_PRINTF("Error Init Accelero Sensor\n\r");
    STBOX1_Error_Handler(STBOX1_ERROR_SENSOR,__FILE__,__LINE__);
  }

  /* Feed the program to Machine Learning Core */
  if((UseCustomIfAvailable==1) & (MLCCustomUCFFile!=NULL)) {
    ProgramPointer    = MLCCustomUCFFile;
    TotalNumberOfLine = MLCCustomUCFFileLength;
    STBOX1_PRINTF("-->Custom UCF Program for ISM330DHCX MLC\r\n");
  } else {
    /* Motion Intensity Default program */
    ProgramPointer = (ucf_line_t *)ism330dhcx_motion_intensity;
    TotalNumberOfLine = sizeof(ism330dhcx_motion_intensity) / sizeof(ucf_line_t);
    STBOX1_PRINTF("-->Activity Recognition for ISM330DHCX_0 MLC\r\n");
    STBOX1_PRINTF("UCF Number Line=%ld\r\n",TotalNumberOfLine);
  }

  for (LineCounter=0; LineCounter<TotalNumberOfLine; LineCounter++) {
    RetValue = BSP_MOTION_SENSOR_Write_Register(ISM330DHCX_0,
                                            ProgramPointer[LineCounter].address,
                                            ProgramPointer[LineCounter].data);
    if(RetValue!=BSP_ERROR_NONE) {
      STBOX1_PRINTF("Error loading the Program to ISM330DHCX_0 [%ld]->%lx\n\r",LineCounter,RetValue);
      STBOX1_Error_Handler(STBOX1_ERROR_SENSOR,__FILE__,__LINE__);
    }
  }

  STBOX1_PRINTF("Program loaded inside the ISM330DHCX MLC\n\r");
}

/** @brief Initialize the ISM330DHCX MEMS Sensor for FSM
 * @param uint32_t UseCustomIfAvailable flag for Using or not the Custom UCF file
 * @retval None
 */
void InitAcc_FSM(uint32_t UseCustomIfAvailable)
{
   ucf_line_t *ProgramPointer;
  int32_t LineCounter;
  int32_t TotalNumberOfLine;
  int32_t RetValue;

  /* Init Accelero */
  if (BSP_MOTION_SENSOR_Init(ISM330DHCX_0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    STBOX1_PRINTF("OK Init Accelero  Sensor\n\r");
  } else {
    STBOX1_PRINTF("Error Init Accelero Sensor\n\r");
    STBOX1_Error_Handler(STBOX1_ERROR_SENSOR,__FILE__,__LINE__);
  }

  /* Feed the program to FiniteStateMachine */
  if((UseCustomIfAvailable==1) & (FSMCustomUCFFile!=NULL)) {
    ProgramPointer    = FSMCustomUCFFile;
    TotalNumberOfLine = FSMCustomUCFFileLength;
    STBOX1_PRINTF("-->Custom UCF Program for ISM330DHCX FSM\r\n");
  } else {
    /* 4D position recognition Default program */
    ProgramPointer = (ucf_line_t *)ism330dhcx_four_d;
    TotalNumberOfLine = sizeof(ism330dhcx_four_d) / sizeof(ucf_line_t);
    STBOX1_PRINTF("-->4D position recognition for ISM330DHCX FSM\r\n");
    STBOX1_PRINTF("UCF Number Line=%ld\r\n",TotalNumberOfLine);
  }

  for (LineCounter=0; LineCounter<TotalNumberOfLine; LineCounter++) {
    RetValue = BSP_MOTION_SENSOR_Write_Register(ISM330DHCX_0,
                                            ProgramPointer[LineCounter].address,
                                            ProgramPointer[LineCounter].data);
    if(RetValue!=BSP_ERROR_NONE) {
      STBOX1_PRINTF("Error loading the Program to ISM330DHCX [%ld]->%lx\n\r",LineCounter,RetValue);
      STBOX1_Error_Handler(STBOX1_ERROR_SENSOR,__FILE__,__LINE__);
    }
  }
  STBOX1_PRINTF("Program loaded inside the ISM330DHCX FSM\n\r");
}

/**
* @brief  Initialize Timers
*
* @param  None
* @retval None
*/
static void InitTimers(void)
{
  uint32_t uwPrescalerValue;

  /* Timer Output Compare Configuration Structure declaration */
  TIM_OC_InitTypeDef sConfig;

  /* Compute the prescaler value to counter clock equal to 10000 Hz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 10000) - 1);

  /* Set TIM1 instance */
  TIM_CC_HANDLE.Instance = TIM_CC_INSTANCE;
  TIM_CC_HANDLE.Init.Period        = 65535;
  TIM_CC_HANDLE.Init.Prescaler     = uwPrescalerValue;
  TIM_CC_HANDLE.Init.ClockDivision = 0;
  TIM_CC_HANDLE.Init.CounterMode   = TIM_COUNTERMODE_UP;

   if(HAL_TIM_OC_DeInit(&TIM_CC_HANDLE) != HAL_OK) {
    /* Initialization Error */
    STBOX1_Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }

  if(HAL_TIM_OC_Init(&TIM_CC_HANDLE) != HAL_OK) {
    /* Initialization Error */
    STBOX1_Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }

  /* Configure the Output Compare channels */

  /* Common configuration for all channels */
  sConfig.OCMode     = TIM_OCMODE_TOGGLE;
  sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

  sConfig.Pulse = STBOX1_UPDATE_LED_BATTERY;
  if(HAL_TIM_OC_ConfigChannel(&TIM_CC_HANDLE, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
    /* Configuration Error */
    STBOX1_Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }

  sConfig.Pulse = STBOX1_UPDATE_INV;
  if(HAL_TIM_OC_ConfigChannel(&TIM_CC_HANDLE, &sConfig, TIM_CHANNEL_3) != HAL_OK) {
    /* Configuration Error */
    STBOX1_Error_Handler(STBOX1_ERROR_HW_INIT,__FILE__,__LINE__);
  }
}

/**
* @brief  Initialize User process
*
* @param  None
* @retval None
*/
static void User_Init(void)
{
  /* Enable VDDA */
  BSP_Enable_LDO();

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

  //Update the Current Fw ID saved in flash if it's neceessary
  UpdateCurrFlashBankFwIdBoardName(STBOX1_BLUEST_SDK_FW_ID,NULL);

  InitTimers();
}

/**
* @brief  Output Compare callback in non blocking mode
* @param  TIM_HandleTypeDef *htim TIM OC handle
* @retval None
*/
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t uhCapture=0;
  /* TIM1_CH1 toggling with frequency = 0.5Hz */
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TIM_CC_HANDLE, TIM_CHANNEL_1, (uhCapture + STBOX1_UPDATE_LED_BATTERY));
    BlinkLed =1;
  }

  /* TIM1_CH2 toggling with frequency = 1Hz */
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
    uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TIM_CC_HANDLE, TIM_CHANNEL_3, (uhCapture + STBOX1_UPDATE_INV));
    SendAccGyroMag=1;
  }
}

/**
* @brief  BSP Push Button callback
*
* @param  Button Specifies the pin connected EXTI line
* @retval None
*/
void BSP_PB_Callback(Button_TypeDef Button)
{
  if(Button == BUTTON_USER) {
    /* Set the User Button flag */
    UserButtonPressed = SET;
  }
}

/**
* @brief  Print Bunner
* @param  None
* @retval None
*/
static void PrintInfo(void)
{
  STBOX1_PRINTF("\r\nSTMicroelectronics %s:\r\n"
            "\tVersion %c.%c.%c\r\n"
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
  * @brief  This function is executed in case of error occurrence.
  * @param int32_t ErrorCode Error Code
  * @retval None
  */
void STBOX1_Error_Handler(int32_t ErrorCode,char *File,int32_t Line)
{
  /* User can add his own implementation to report the HAL error return state */
  BSP_LED_Off(LED_ORANGE);
  STBOX1_PRINTF("Error at %ld at %s\r\n",Line,File);
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

/**
 * @brief  Initialize the LDO: analog power supply.
 * @param  None
 * @retval None
 */
void BSP_Enable_LDO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* Configure the DCDC2 Enable pin */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
}

/**
 * @brief  DeInitialize the DCDC MSP.
 * @param  None
 * @retval None
 */
void BSP_Disable_LDO(void)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

HAL_StatusTypeDef MX_ACC_SPI_Init(SPI_HandleTypeDef* hspi)
{
  HAL_StatusTypeDef ret = HAL_OK;
  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  hspi->Instance = HANDLE_ACC_SPI;
  hspi->Init.Mode = SPI_MODE_MASTER;
  hspi->Init.Direction = SPI_DIRECTION_2LINES;
  hspi->Init.DataSize = SPI_DATASIZE_8BIT;
  hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi->Init.NSS = SPI_NSS_SOFT;
  hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi->Init.TIMode = SPI_TIMODE_DISABLE;
  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi->Init.CRCPolynomial = 0x7;
  hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi->Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi->Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi->Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi->Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(hspi) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH1_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(hspi, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  __HAL_SPI_ENABLE(hspi);

  return ret;
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

