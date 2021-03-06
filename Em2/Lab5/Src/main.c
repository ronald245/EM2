/**
  ******************************************************************************
  * @file    COMP/COMP_Interrupt/Src/main.c
  * @author  MCD Application Team
  * @brief   This example provides a short description of how to use the COMP 
  *          peripheral Interrupt.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L1xx_HAL_Examples
  * @{
  */

/** @addtogroup COMP_Interrupt
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
COMP_HandleTypeDef   Comp1Handle;
TIM_HandleTypeDef htim;	   
int rotations;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Error_Handler(void);
static void COMP_Config(void);
static void TIM_Config(void);
static void Hall_Config(void);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

	/* Configure the system clock to 32 MHz */
	SystemClock_Config();

	BSP_LCD_GLASS_Init();
	HAL_Init();
	TIM_Config();	
	Hall_Config();
	

	/* Configure LED3 */
	BSP_LED_Init(LED3);
  
	/*## Configure peripherals #################################################*/
	/* Configure the COMP1 peripheral */
	COMP_Config();
  
	/*## Enable peripherals ####################################################*/
	/* Start COMP1 */
	if (HAL_COMP_Start_IT(&Comp1Handle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler(); 
	}
	/* Infinite loop */
	while (1)
	{
	}
}

static void Hall_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;

	GPIO_InitStructure.Pin = GPIO_PIN_0;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	   	
	/* Tell system that you will use PC0 for EXTI_Line0 */ 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC , EXTI_PinSource0); 

	/* Pc0 is connected to EXTI_Line0 , which has EXTI0_IRQn vector */ 
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn; /* Set priority */ 
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00; /* Set sub priority */ 
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00; /* Enable interrupt */ 
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; /* Add to NVIC */ 
	NVIC_Init(&NVIC_InitStruct);

}


static void TIM_Config()
{
	TIM_Base_InitTypeDef  tbs;																																						  
	tbs.Prescaler = 32;    // clock 16MHz divide by 32 -> 500 kHz 
	tbs.Period = 500000;   // 500khz / 500k = 1hz
	tbs.CounterMode = TIM_COUNTERMODE_UP;
	tbs.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	htim.Channel = HAL_TIM_ACTIVE_CHANNEL_2;
	htim.Init  = tbs;
	htim.Instance = TIM2;
	
	HAL_TIM_Base_Init(&htim);
	HAL_TIM_Base_Start_IT(&htim);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef * htim)
{
	if (htim->Instance == TIM2)
	{

		char displayString[20];
		int rpm = rotations * 60;
		sprintf(displayString, "%02d RPM", rpm);

		BSP_LCD_GLASS_DisplayString(displayString);
		rotations = 0;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0)
	{
		rotations++;

	}
}


/**
  * @brief  COMP configuration		  1
  * @param  None
  * @retval None
  */
static void COMP_Config(void)
{
	/*##-1- Configure the COMP1 peripheral ###################################*/
	Comp1Handle.Instance = COMP1;
	/* COMP1 Init: the higher threshold is fixed by hardware to VREFINT         */
	/* (~ 1.22V).                                                               */
	Comp1Handle.Instance                   = COMP1;

	Comp1Handle.Init.NonInvertingInput     = COMP_NONINVERTINGINPUT_PA1;
	Comp1Handle.Init.NonInvertingInputPull = COMP_NONINVERTINGINPUT_NOPULL;
	Comp1Handle.Init.TriggerMode           = COMP_TRIGGERMODE_IT_RISING_FALLING;
	if (HAL_COMP_Init(&Comp1Handle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
}

/**
  * @brief  Comparator interrupt callback.
  * @param  hcomp: COMP handle
  * @retval None
  */
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{
	/* Toggle LED3 */
	BSP_LED_Toggle(LED3);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLLMUL                         = 6
  *            PLLDIV                         = 3
  *            Flash Latency(WS)              = 1
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };

	/* Enable HSE Oscillator and Activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLL_DIV3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		/* Initialization Error */
		while (1) ; 
	}

	/* Set Voltage scale1 as MCU will run at 32MHz */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
	/* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
	while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {}
	;

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		/* Initialization Error */
		while (1) ; 
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  
	while (1)
	{
		/* Toggle LED3 */
		BSP_LED_Toggle(LED3);
      
		/* Add a delay */
		HAL_Delay(100);
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

	 /* Infinite loop */
	while (1)
	{
	}
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
