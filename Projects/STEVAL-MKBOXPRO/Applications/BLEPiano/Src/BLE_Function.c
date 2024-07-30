/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   BLEPiano\Src\BLE_Function.c
  * @author System Research & Applications Team - Agrate/Catania Lab.
  * @brief  Implementation of API called from BLE Manager
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
#include <stdio.h>
#include "STBOX1_config.h"
#include "BLE_Manager.h"
#include "main.h"
#include "OTA.h"
#include "note.h"
#include "noteCodeTable.h"
#include "steval_mkboxpro.h"
#include "BLE_Function.h"
#include "app_blepiano.h"

/* Exported Variables --------------------------------------------------------*/
volatile uint8_t  connected   = FALSE;
volatile uint32_t RebootBoard = 0;
volatile uint32_t SwapBanks   = 0;
uint32_t ConnectionBleStatus =0;
uint32_t SizeOfUpdateBlueFW=0;

/* Imported Variables --------------------------------------------------------*/
extern uint8_t BatteryPresent;

/* Private variables ---------------------------------------------------------*/
static uint32_t NeedToRebootBoard=0;
static uint32_t NeedToSwapBanks=0;

/* Private functions ---------------------------------------------------------*/
uint32_t DebugConsoleParsing(uint8_t * att_data, uint8_t data_length);
void DisconnectionCompletedFunction(void);
void ConnectionCompletedFunction(uint16_t ConnectionHandle, uint8_t Address_Type, uint8_t Addr[6]);
uint32_t DebugConsoleCommandParsing(uint8_t * att_data, uint8_t data_length);

void WriteRequestPianoFunction(uint8_t * att_data, uint8_t data_length);

/**********************************************************************************************
 * Callback functions prototypes to manage the extended configuration characteristic commands *
 **********************************************************************************************/
void ExtExtConfigUidCommandCallback(uint8_t **UID);
void ExtConfigInfoCommandCallback(uint8_t *Answer);
void ExtConfigHelpCommandCallback(uint8_t *Answer);
void ExtConfigVersionFwCommandCallback(uint8_t *Answer);
void ExtConfigSetNameCommandCallback(uint8_t *NewName);

void NotifyEventBattery(BLE_NotifyEvent_t Event);
void NotifyEventPiano(BLE_NotifyEvent_t Event);

/**
 * @brief  Set Custom Advertise Data.
 * @param  uint8_t *manuf_data: Advertise Data
 * @retval None
 */
void BLE_SetCustomAdvertiseData(uint8_t *manuf_data)
{
  if(FinishGood==FINISHA) {
    BLE_StackValue.BoardId = BLE_MANAGER_SENSOR_TILE_BOX_PRO_PLATFORM;
    manuf_data[BLE_MANAGER_CUSTOM_FIELD1]=STBOX1A_BLUEST_SDK_FW_ID;
  } else {
    BLE_StackValue.BoardId = BLE_MANAGER_SENSOR_TILE_BOX_PRO_B_PLATFORM;
    manuf_data[BLE_MANAGER_CUSTOM_FIELD1]=STBOX1B_BLUEST_SDK_FW_ID;
  }

  manuf_data[BLE_MANAGER_CUSTOM_FIELD1 - 1]= BLE_StackValue.BoardId;
  manuf_data[BLE_MANAGER_CUSTOM_FIELD2]=CurrentActiveBank;
  manuf_data[BLE_MANAGER_CUSTOM_FIELD3]=0x00; /* Not Used */
  manuf_data[BLE_MANAGER_CUSTOM_FIELD4]=0x00; /* Not Used */
}

/**
 * @brief  Callback Function for Un/Subscription Battery Feature
 * @param  BLE_NotifyEvent_t Event Sub/Unsub
 * @retval None
 */
void NotifyEventBattery(BLE_NotifyEvent_t Event)
{
  if(Event == BLE_NOTIFY_SUB){
    uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TIM_CC_HANDLE);

    W2ST_ON_CONNECTION(W2ST_CONNECT_BAT_EVENT);

    /* Start the TIM Base generation in interrupt mode */
    if(HAL_TIM_OC_Start_IT(&TIM_CC_HANDLE, TIM_CHANNEL_1) != HAL_OK){
      /* Starting Error */
      STBOX1_Error_Handler(STBOX1_ERROR_TIMER,__FILE__,__LINE__);
    }

    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TIM_CC_HANDLE, TIM_CHANNEL_1, (uhCapture + STBOX1_UPDATE_LED_BATTERY));
      STBOX1_PRINTF("Start Battery\r\n");
  } else if(Event == BLE_NOTIFY_UNSUB) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_BAT_EVENT);

    /* Stop the TIM Base generation in interrupt mode */
    if(HAL_TIM_OC_Stop_IT(&TIM_CC_HANDLE, TIM_CHANNEL_1) != HAL_OK){
      /* Stopping Error */
      STBOX1_Error_Handler(STBOX1_ERROR_TIMER,__FILE__,__LINE__);
    }
    STBOX1_PRINTF("Stop Battery\r\n");
  }
}

/**
 * @brief  Callback Function for Un/Subscription Piano Feature
 * @param  BLE_NotifyEvent_t Event Sub/Unsub
 * @retval None
 */
void NotifyEventPiano(BLE_NotifyEvent_t Event)
{
  if(Event == BLE_NOTIFY_SUB){

    W2ST_ON_CONNECTION(W2ST_CONNECT_PIANO_EVENT);

    TIM_Beep_Init();
    STBOX1_PRINTF("Start Piano\r\n");
  } else if(Event == BLE_NOTIFY_UNSUB) {
    W2ST_OFF_CONNECTION(W2ST_CONNECT_PIANO_EVENT);

    TIM_Beep_DeInit();
    InitTimers();
    STBOX1_PRINTF("Stop Piano\r\n");
  }
}

/**
 * @brief  This function makes the parsing of the Piano Commands
 * @param  uint8_t *att_data attribute data
 * @param  uint8_t data_length length of the data
 * @retval None
 */
void WriteRequestPianoFunction(uint8_t * att_data, uint8_t data_length)
{
#define PIANO_OCTAVE 3
  if(att_data[0] == BLE_PIANO_START) {
    if(att_data[1]<MAX_NOTE_NUM) {
      beep(NoteArray[att_data[1]+(PIANO_OCTAVE*12)],0 /* For Avoiding to stop the Note */);
    } else {
      STBOX1_PRINTF("Error Piano Feature Unsupported Note =%d\r\n",att_data[1]);
    }
  } else if(att_data[0] == BLE_PIANO_STOP) {
    beep(0,0 /* Not important */);
  } else {
    STBOX1_PRINTF("Error Piano Feature Unsupported Command =%x\r\n",att_data[0]);
  }
#undef PIANO_OCTAVE
}

/**
* @brief  This function makes the parsing of the Debug Console
* @param  uint8_t *att_data attribute data
* @param  uint8_t data_length length of the data
* @retval uint32_t SendBackData true/false
*/
uint32_t DebugConsoleParsing(uint8_t * att_data, uint8_t data_length)
{
  /* By default Answer with the same message received */
  uint32_t SendBackData =1;

 if(SizeOfUpdateBlueFW!=0) {
    /* Firwmare update */
    int8_t RetValue = UpdateFWBlueMS(&SizeOfUpdateBlueFW,att_data, data_length,1);
    if(RetValue!=0) {
      Term_Update(((uint8_t *)&RetValue),1);
      if(RetValue==1) {
        /* if OTA checked */
        STBOX1_PRINTF("%s will restart after the disconnection\r\n",STBOX1_PACKAGENAME);
        HAL_Delay(1000);
        NeedToSwapBanks=1;
      }
    }
    SendBackData=0;
  } else {
    /* Received one write from Client on Terminal characteristc */
    SendBackData = DebugConsoleCommandParsing(att_data,data_length);
  }

  return SendBackData;
}

/**
 * @brief  This function makes the parsing of the Debug Console Commands
 * @param  uint8_t *att_data attribute data
 * @param  uint8_t data_length length of the data
 * @retval uint32_t SendBackData true/false
 */
uint32_t DebugConsoleCommandParsing(uint8_t * att_data, uint8_t data_length)
{
  uint32_t SendBackData = 1;

  /* Help Command */
  if(!strncmp("help",(char *)(att_data),4)) {
    /* Print Legend */
    SendBackData=0;

    BytesToWrite =sprintf((char *)BufferToWrite,
                          "info\n");
    Term_Update(BufferToWrite,BytesToWrite);
  } else if(!strncmp("versionFw",(char *)(att_data),9)) {
    BytesToWrite =sprintf((char *)BufferToWrite,"%s_%s_%c.%c.%c\r\n",
                          "U585",
                          STBOX1_PACKAGENAME,
                          STBOX1_VERSION_MAJOR,
                          STBOX1_VERSION_MINOR,
                          STBOX1_VERSION_PATCH);
    Term_Update(BufferToWrite,BytesToWrite);
    SendBackData=0;
  } else if(!strncmp("info",(char *)(att_data),4)) {
    SendBackData=0;

    BytesToWrite =sprintf((char *)BufferToWrite,"\r\nSTMicroelectronics %s:\n"
       "\tVersion %c.%c.%c\n"
      "\tSTM32U585AI-SensorTile.box-Pro  (%c) board"
        "\n",
        STBOX1_PACKAGENAME,
        STBOX1_VERSION_MAJOR,STBOX1_VERSION_MINOR,STBOX1_VERSION_PATCH,
        (FinishGood==FINISHA) ? 'A' : 'B');
    Term_Update(BufferToWrite,BytesToWrite);

    BytesToWrite =sprintf((char *)BufferToWrite,"\t(HAL %ld.%ld.%ld_%ld)\n"
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
    Term_Update(BufferToWrite,BytesToWrite);
    BytesToWrite =sprintf((char *)BufferToWrite,"Current Bank =%ld\n",CurrentActiveBank);
    Term_Update(BufferToWrite,BytesToWrite);
  } else if(!strncmp("upgradeFw",(char *)(att_data),9)) {
    uint32_t uwCRCValue;
    uint8_t *PointerByte = (uint8_t*) &SizeOfUpdateBlueFW;

    PointerByte[0]=att_data[ 9];
    PointerByte[1]=att_data[10];
    PointerByte[2]=att_data[11];
    PointerByte[3]=att_data[12];

    /* Check the Maximum Possible OTA size */
    if(SizeOfUpdateBlueFW>OTA_MAX_PROG_SIZE) {
      STBOX1_PRINTF("OTA %s SIZE=%ld > %d Max Allowed\r\n",STBOX1_PACKAGENAME,SizeOfUpdateBlueFW, OTA_MAX_PROG_SIZE);
      /* Answer with a wrong CRC value for signaling the problem to BlueMS application */
      BufferToWrite[0]= att_data[13];
      BufferToWrite[1]=(att_data[14]!=0) ? 0 : 1;/* In order to be sure to have a wrong CRC */
      BufferToWrite[2]= att_data[15];
      BufferToWrite[3]= att_data[16];
      BytesToWrite = 4;
      Term_Update(BufferToWrite,BytesToWrite);
    } else {
      PointerByte = (uint8_t*) &uwCRCValue;
      PointerByte[0]=att_data[13];
      PointerByte[1]=att_data[14];
      PointerByte[2]=att_data[15];
      PointerByte[3]=att_data[16];

      STBOX1_PRINTF("OTA %s SIZE=%ld uwCRCValue=%lx\r\n",STBOX1_PACKAGENAME,SizeOfUpdateBlueFW,uwCRCValue);

      /* Reset the Flash */
      StartUpdateFWBlueMS(SizeOfUpdateBlueFW,uwCRCValue);

#if 0
      /* Reduce the connection interval */
      {
         tBleStatus ret = aci_l2cap_connection_parameter_update_req(
                                                      ConnectionHandle,
                                                      6 /* interval_min*/,
                                                      6 /* interval_max */,
                                                      0   /* slave_latency */,
                                                      400 /*timeout_multiplier*/);
        /* Go to infinite loop if there is one error */
        if (ret != BLE_STATUS_SUCCESS) {
          while (1) {
            STBOX1_PRINTF("Problem Changing the connection interval\r\n");
          }
        }
      }
#endif

      /* Signal that we are ready sending back the CRV value*/
      BufferToWrite[0] = PointerByte[0];
      BufferToWrite[1] = PointerByte[1];
      BufferToWrite[2] = PointerByte[2];
      BufferToWrite[3] = PointerByte[3];
      BytesToWrite = 4;
      Term_Update(BufferToWrite,BytesToWrite);
    }

    SendBackData=0;
  }  else if(!strncmp("uid",(char *)(att_data),3)) {
    /* Write back the STM32 UID */
    uint8_t *uid = (uint8_t *)STM32_UUID;
    uint32_t MCU_ID = STM32_MCU_ID[0]&0xFFF;
    BytesToWrite =sprintf((char *)BufferToWrite,"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X_%.3lX\n",
                          uid[ 3],uid[ 2],uid[ 1],uid[ 0],
                          uid[ 7],uid[ 6],uid[ 5],uid[ 4],
                          uid[11],uid[ 10],uid[9],uid[8],
                          MCU_ID);
    Term_Update(BufferToWrite,BytesToWrite);
    SendBackData=0;
  }

  return SendBackData;
}

/**
 * @brief  This function is called when the peer device get disconnected.
 * @param  None
 * @retval None
 */
void DisconnectionCompletedFunction(void)
{
  connected = FALSE;

  /* Make the device connectable again */

  /* Reset for any problem during FOTA update */
  SizeOfUpdateBlueFW = 0;

  /*Stop all the timers */
  if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_BAT_EVENT)) {
    if(HAL_TIM_OC_Stop_IT(&TIM_CC_HANDLE, TIM_CHANNEL_1) != HAL_OK){
      /* Stopping Error */
      STBOX1_Error_Handler(STBOX1_ERROR_TIMER,__FILE__,__LINE__);
    }
    STBOX1_PRINTF("Stop Battery\r\n");
  }

   if(W2ST_CHECK_CONNECTION(W2ST_CONNECT_PIANO_EVENT)) {
    TIM_Beep_DeInit();
    InitTimers();
    STBOX1_PRINTF("Stop Piano\r\n");
  }

  /* Reset the BLE Connection Variable */
  ConnectionBleStatus=0;

  if(NeedToRebootBoard) {
    NeedToRebootBoard=0;
    RebootBoard=1;
  }

  if(NeedToSwapBanks) {
    NeedToSwapBanks=0;
    SwapBanks=1;
  }
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  None
 * @retval None
 */
void ConnectionCompletedFunction(uint16_t ConnectionHandle, uint8_t Address_Type,uint8_t Addr[6])
{
  connected = TRUE;
  ConnectionBleStatus=0;

  /* Stop the TIM Base generation in interrupt mode for Led Blinking*/
  if(HAL_TIM_OC_Stop_IT(&TIM_CC_HANDLE, TIM_CHANNEL_1) != HAL_OK){
    /* Stopping Error */
    STBOX1_Error_Handler(STBOX1_ERROR_TIMER,__FILE__,__LINE__);
  }

  BSP_LED_Off(LED_GREEN);

  TIM_Beep_DeInit();
  HAL_Delay(100);
}

/**
 * @brief  Enable Disable the jump to second flash bank and reboot board
 * @param  None
 * @retval None
 */
void EnableDisableDualBoot(void)
{
  FLASH_OBProgramInitTypeDef    OBInit;
  /* Set BFB2 bit to enable boot from Flash Bank2 */
  /* Allow Access to Flash control registers and user Flash */
  HAL_FLASH_Unlock();

  /* Allow Access to option bytes sector */
  HAL_FLASH_OB_Unlock();

  /* Get the Dual boot configuration status */
  HAL_FLASHEx_OBGetConfig(&OBInit);

  /* Enable/Disable dual boot feature */
  OBInit.OptionType = OPTIONBYTE_USER;
  OBInit.USERType   = OB_USER_SWAP_BANK;

  if (((OBInit.USERConfig) & (FLASH_OPTR_SWAP_BANK)) == FLASH_OPTR_SWAP_BANK) {
    OBInit.USERConfig &= ~FLASH_OPTR_SWAP_BANK;
    STBOX1_PRINTF("->Disable DualBoot\r\n");
  } else {
    OBInit.USERConfig = FLASH_OPTR_SWAP_BANK;
    STBOX1_PRINTF("->Enable DualBoot\r\n");
  }

  if(HAL_FLASHEx_OBProgram (&OBInit) != HAL_OK) {
    /*
    Error occurred while setting option bytes configuration.
    User can add here some code to deal with this error.
    To know the code error, user can call function 'HAL_FLASH_GetError()'
    */
    STBOX1_Error_Handler(STBOX1_ERROR_FLASH,__FILE__,__LINE__);
  }

  /* Start the Option Bytes programming process */
  if (HAL_FLASH_OB_Launch() != HAL_OK) {
    /*
    Error occurred while reloading option bytes configuration.
    User can add here some code to deal with this error.
    To know the code error, user can call function 'HAL_FLASH_GetError()'
    */
    STBOX1_Error_Handler(STBOX1_ERROR_FLASH,__FILE__,__LINE__);
  }
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();
}

/***********************************************************************************
 * Callback functions to manage the extended configuration characteristic commands *
 ***********************************************************************************/

/**
 * @brief  Callback Function for answering to the UID command
 * @param  uint8_t **UID STM32 UID Return value
 * @retval None
 */
void ExtExtConfigUidCommandCallback(uint8_t **UID)
{
  *UID = (uint8_t *)STM32_UUID;
}

/**
 * @brief  Callback Function for answering to Info command
 * @param  uint8_t *Answer Return String
 * @retval None
 */
void ExtConfigInfoCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer,"\r\nSTMicroelectronics %s:\n"
       "\tVersion %c.%c.%c\n"
      "\tSTM32U585AI-SensorTile.box-Pro (%c) board"
      "\n\t(HAL %ld.%ld.%ld_%ld)\n"
      "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
      " (IAR)\n"
#elif defined (__ARMCC_VERSION)
       " (KEIL)\n"
#elif defined (__GNUC__)
       " (STM32CubeIDE)\n"
#endif
       "\tCurrent Bank =%ld\n",
        STBOX1_PACKAGENAME,
        STBOX1_VERSION_MAJOR,STBOX1_VERSION_MINOR,STBOX1_VERSION_PATCH,
        (FinishGood==FINISHA) ? 'A' : 'B',
        HAL_GetHalVersion() >>24,
        (HAL_GetHalVersion() >>16)&0xFF,
        (HAL_GetHalVersion() >> 8)&0xFF,
         HAL_GetHalVersion()      &0xFF,
         __DATE__,__TIME__,
         CurrentActiveBank);
}

/**
 * @brief  Callback Function for answering to SetName command
 * @param  uint8_t *NewName New Name
 * @retval None
 */
void ExtConfigSetNameCommandCallback(uint8_t *NewName)
{
  STBOX1_PRINTF("Received a new Board's Name=%s\r\n",NewName);
  /* Update the Board's name in flash */
  if(FinishGood==FINISHA) {
    UpdateCurrFlashBankFwIdBoardName(STBOX1A_BLUEST_SDK_FW_ID,NewName);
  } else {
    UpdateCurrFlashBankFwIdBoardName(STBOX1B_BLUEST_SDK_FW_ID,NewName);
  }

  /* Update the Name for BLE Advertise */
  sprintf(BLE_StackValue.BoardName,"%s",NewName);
}

/**
 * @brief  Callback Function for answering to Help command
 * @param  uint8_t *Answer Return String
 * @retval None
 */
void ExtConfigHelpCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer,"Help Message.....");
}

/**
 * @brief  Callback Function for answering to VersionFw command
 * @param  uint8_t *Answer Return String
 * @retval None
 */
void ExtConfigVersionFwCommandCallback(uint8_t *Answer)
{
  sprintf((char *)Answer,"%s_%s_%c.%c.%c\r\n",
                  "U585",
                  STBOX1_PACKAGENAME,
                  STBOX1_VERSION_MAJOR,
                  STBOX1_VERSION_MINOR,
                  STBOX1_VERSION_PATCH);
}

/**
 * @brief  Callback Function for answering to ReadBanksFwId command
 * @param  uint8_t *CurBank Number Current Bank
 * @param  uint16_t *FwId1 Bank1 Firmware Id
 * @param  uint16_t *FwId2 Bank2 Firmware Id
 * @retval None
 */
void ExtConfigReadBanksFwIdCommandCallback (uint8_t *CurBank,uint16_t *FwId1,uint16_t *FwId2)
{
  ReadFlashBanksFwId(FwId1,FwId2);
  *CurBank=CurrentActiveBank;
}

/**
 * @brief  Callback Function for answering to BanksSwap command
 * @param  None
 * @retval None
 */
void ExtConfigBanksSwapCommandCallback(void)
{
  uint16_t FwId1,FwId2;

  ReadFlashBanksFwId(&FwId1,&FwId2);
  if(FwId2!=OTA_OTA_FW_ID_NOT_VALID) {
    STBOX1_PRINTF("Swapping to Bank%d\n",(CurrentActiveBank==1) ? 0 : 1);
    STBOX1_PRINTF("%s will restart after the disconnection\r\n",STBOX1_PACKAGENAME);
    NeedToSwapBanks = 1;
  } else {
    STBOX1_PRINTF("Not Valid fw on Bank%d\n\tCommand Rejected\n",(CurrentActiveBank==1) ? 0 : 1);
    STBOX1_PRINTF("\tLoad a Firwmare on Bank%d\n",(CurrentActiveBank==1) ? 0 : 1);
  }
}

/**
  * @brief  Set Board Name.
  * @param  None
  * @retval None
  */
void SetBoardName(void)
{
  sprintf(BLE_StackValue.BoardName, "%s", STBOX1_FW_PACKAGENAME);
}

