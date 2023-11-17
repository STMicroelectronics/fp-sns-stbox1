/**
  ******************************************************************************
  * @file    BLELowPowerRToS\Src\PowerControl.c
  * @author  System Research & Applications Team - Agrate/Catania Lab.
  * @version V1.6.0
  * @date    20-October-2023
  * @brief   This file provide the power managing to reduce the power consump
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "PowerControl.h"
#include "cmsis_os.h"
#include "TargetFeatures.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define RTC_ASYNCH_PREDIV    0x0
#define RTC_SYNCH_PREDIV     0x7FFF

/* Imported Variables -------------------------------------------------------------*/
/* Imported for IT handler */
extern RTC_HandleTypeDef RtcHandle;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SYSCLKConfig_STOP(void);

static powerState_t minPowerMode =  RUN;
static int PowerCtrlLockToken =  0;
static RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
static RCC_OscInitTypeDef RCC_OscInitStruct = {0};
static RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
static uint32_t pFLatency = 0;

powerState_t GetMinPowerMode(void)
{
  return minPowerMode;
}

int32_t SetMinPowerMode(powerState_t powerMode)
{
  minPowerMode = powerMode ;
  return 0;
}

int32_t PowerCtrlLock(void)
{
  return ++PowerCtrlLockToken;
}
int32_t PowerCtrlUnLock(void)
{
  PowerCtrlLockToken -= (PowerCtrlLockToken) ? 1 : 0;
  return PowerCtrlLockToken;
}
int32_t PowerCtrlGetState(void)
{
  return PowerCtrlLockToken;
}

void vApplicationIdleHook(void)
{
  __WFI();
}

int32_t initPowerController(void)
{
  SetMinPowerMode(RUN);

  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  /* Ensure that MSI is wake-up system clock */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

  /* Configure RTC */
  RtcHandle.Instance = RTC;
  /* Set the RTC time base to 1s */
  /* Configure RTC prescaler and RTC data registers as follow:
    - Hour Format = Format 24
    - Asynch Prediv = Value according to source clock
    - Synch Prediv = Value according to source clock
    - OutPut = Output Disable
    - OutPutPolarity = High Polarity
    - OutPutType = Open Drain */
  RtcHandle.Init.HourFormat     = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv   = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv    = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    while (1);
  }
  return 0 ;
}

uint32_t timeInSeconds(uint32_t inBCD)
{
  uint32_t timeOut;
  timeOut  = ((inBCD & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos);       /*  seconds */
  timeOut += ((inBCD & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos) * 10;  /* tens of seconds */
  timeOut += ((inBCD & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos) * 60; /* minutes */
  timeOut += ((inBCD & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos) * 60 * 10; /* tens of minutes */
  timeOut += ((inBCD & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos) * 60 * 60; /* hours */
  timeOut += ((inBCD & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos) * 10 * 60 * 60; /* tens of hours */
  timeOut += ((inBCD & RTC_TR_PM_Msk) >> RTC_TR_PM_Pos) * 12 * 60 * 60; /* PM */
  return timeOut;
}

void wakeIn(uint32_t waketime)
{
  HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
  if (waketime < 32000)
  {
    /* resolution of 1/2048 = 0,48828125 ms */
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, (waketime << 11) / 1000, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
  }
  else
  {
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, 0xFFFF, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
    /* later, would set ALARM A or B with 1 second resolution instead */
  }
  /* STBOX1_PRINTF("(%d)",waketime); */
}

extern __IO uint32_t uwTick; /* ?? needed */

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
  eSleepModeStatus eSleepStatus;
  int32_t tickDif;
  int32_t secDif;
  uint32_t secIn;
  uint32_t secOut;
  uint32_t tickIn;
  uint32_t tickOut;
  uint32_t missedTicks;
  volatile uint32_t DR;
  volatile uint32_t TRin;
  volatile uint32_t TRout;

#ifdef DISABLE_PM
  minPowerMode = IDLE_WFI;
#endif /* DISABLE_PM */

  if ((minPowerMode < IDLE_WFI_TICK_SUPRESS) || (PowerCtrlLockToken))
  {
    return;
  }

  tickIn = RTC->SSR;
  HAL_SuspendTick();
  TRin = RTC->TR & RTC_TR_RESERVED_MASK;
  DR     = RTC->DR & RTC_DR_RESERVED_MASK;; /* unlock shadow registers */
  UNUSED(DR);

  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  /* Get the Clocks configuration according to the internal RCC registers */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
  /* Get the Peripheral Clocks configuration according to the internal RCC registers */
  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInit);

  /* Ensure it is still ok to enter the sleep mode. */
  eSleepStatus = eTaskConfirmSleepModeStatus();

  if (eSleepStatus != eAbortSleep)
  {
    if (eSleepStatus != eNoTasksWaitingTimeout)
    {
      /* It is necessary to configure an interrupt to bring the
      microcontroller out of its low power state at a fixed time in the
      future. */
      /* Configure an interrupt to bring the microcontroller out of its low
      power state at the time the kernel next needs to execute.  The
      interrupt must be generated from a source that remains operational
      when the microcontroller is in a low power state. */
      /* wakeIn(xExpectedIdleTime-1); */
      /* configEXPECTED_IDLE_TIME_BEFORE_SLEEP >=3 */
      configASSERT(configEXPECTED_IDLE_TIME_BEFORE_SLEEP > 2);
      wakeIn(xExpectedIdleTime - 2);
    }
    __asm volatile("cpsid i");
    __asm volatile("dsb");
    __asm volatile("isb");
    if (minPowerMode >= IDLE_SLEEP_STOP)
    {
      /* Enter STOP 2 mode */
      HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
      /* Re-configure the system clock, enable and
       select PLL as system clock source (PLL is disabled in STOP mode) */
      SYSCLKConfig_STOP();
    }
    else
    {
      __WFI();
    }
    __asm volatile("cpsie i");
    __asm volatile("dsb");
    __asm volatile("isb");

  }
  /* Determine how long the microcontroller was actually in a low power
  state for, which will be less than xExpectedIdleTime if the
  microcontroller was brought out of low power mode by an interrupt
  other than that configured by the vSetWakeTimeInterrupt() call.
  Note that the scheduler is suspended before
  portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
  portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
  execute until this function completes. */
  tickOut = RTC->SSR;
  TRout   = RTC->TR & RTC_TR_RESERVED_MASK;
  DR      = RTC->DR & RTC_DR_RESERVED_MASK; /* unlock shadow registers & check day wrap around */
  UNUSED(DR);

  secIn    = timeInSeconds(TRin);
  secOut   = timeInSeconds(TRout);
  secDif   = secOut - secIn ;
  tickDif  = tickIn - tickOut ; /* sub second register is a decreasing counter */
  if (secDif)
  {
    secDif  += (secOut < secIn) ? 24 * 60 * 60  : 0 ;
    tickDif += RTC_SYNCH_PREDIV;
    tickDif += (secDif - 1) << 15;
  }
  missedTicks = (int)(((float)tickDif) / 32.768f);
  uwTick += missedTicks;
  /* Correct the kernels tick count to account for the time the
  microcontroller spent in its low power state. */
  vTaskStepTick(missedTicks);

  /* Restart the timer that is generating the tick interrupt. */
  HAL_ResumeTick();
  HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
#ifdef DEBUG_PM
  STBOX1_PRINTF("zz %d ms\r\n", missedTicks);
#endif /* DEBUG_PM */
}

/**
  * @brief  Configures system clock after wake-up from STOP: enable MSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
static void SYSCLKConfig_STOP(void)
{
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while (1);
  }
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
  {
    while (1);
  }
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    while (1);
  }
}
