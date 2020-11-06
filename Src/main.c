/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "cmsis_os.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "freeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//
//Create Task defines
TaskHandle_t INPUTCAPTUREhandler;
void INPUTCAPTURE_Task(void *pvParameters);

TaskHandle_t MODBUShandler;
void MODBUS_Task(void *pvParameters);

TaskHandle_t ADChandler;
void ADC_Task(void *pvParameters);

TaskHandle_t RECEIVEhandler;
void RECEIVE_Task(void *pvParameters);

TaskHandle_t MQTThandler;
void MQTT_Task(void *pvParameters);

TaskHandle_t LWIPhandler;
void LWIP_Task(void *pvParameters);

// semaphore related
SemaphoreHandle_t ADCSem;
SemaphoreHandle_t ModBusSem;
SemaphoreHandle_t InputCaptureSem;
SemaphoreHandle_t LWIPSem;
SemaphoreHandle_t SendingSem;
SemaphoreHandle_t UartSem;

int check =0;
double adcVol=0;
// uart related
uint8_t rx_data = 0;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_IT(&hadc1);
	
	
	HAL_UART_Receive_IT(&huart3, &rx_data, 1);

 ModBusSem = xSemaphoreCreateCounting(1,0);
 InputCaptureSem = xSemaphoreCreateCounting(1,0);
 LWIPSem = xSemaphoreCreateCounting(1,0);
 ADCSem = xSemaphoreCreateCounting(1,0);
 SendingSem = xSemaphoreCreateCounting(1,0);
 
  if (ModBusSem == NULL)HAL_UART_Transmit(&huart3, (uint8_t *) "Unable to Create ModBusSem\n\n", 28, 100);
 else HAL_UART_Transmit(&huart3, (uint8_t *) "ModBusSem Semaphore Created successfully\n\n", 41, 1000);
 
  if (InputCaptureSem == NULL)HAL_UART_Transmit(&huart3, (uint8_t *) "Unable to Create InputCaptureSem\n\n", 100, 1000);
 else HAL_UART_Transmit(&huart3, (uint8_t *) "InputCaptureSem Semaphore Created successfully\n\n", 100, 1000);
 
 if (LWIPSem == NULL)HAL_UART_Transmit(&huart3, (uint8_t *) "Unable to Create LWIPSem\n\n", 28, 1000);
 else HAL_UART_Transmit(&huart3, (uint8_t *) "LWIPSem Semaphore Created successfully\n\n", 41, 1000);
 
 if (ADCSem == NULL)HAL_UART_Transmit(&huart3, (uint8_t *) "Unable to Create ADCSem\n\n", 28, 1000);
 else HAL_UART_Transmit(&huart3, (uint8_t *) "ADCSem Semaphore Created successfully\n\n", 41, 1000);
 
 if (SendingSem == NULL)HAL_UART_Transmit(&huart3, (uint8_t *) "Unable to Create SendingSem\n\n", 28, 1000);
 else HAL_UART_Transmit(&huart3, (uint8_t *) "SendingSem Semaphore Created successfully\n\n", 50, 1000);
 
 // create Tasks
 
 xTaskCreate(INPUTCAPTURE_Task, "INPUTCAPTURE", 128, NULL, 5, &INPUTCAPTUREhandler);
 xTaskCreate(MODBUS_Task, "MODBUS", 128, NULL, 2, &MODBUShandler);
 xTaskCreate(ADC_Task, "ADC", 128, NULL, 3, &ADChandler);
 //xTaskCreate(RECEIVE_Task, "RECEIVE", 128, NULL, 1, &RECEIVEhandler);
 xTaskCreate(MQTT_Task, "MQTT", 128, NULL, 1, &MQTThandler);
 xTaskCreate(LWIP_Task, "LWIP", 128, NULL, 4, &LWIPhandler);
 
 vTaskStartScheduler();
 
  /* USER CODE END 2 */




  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void INPUTCAPTURE_Task(void *pvParameters)
{
	

 // Give  semaphores at the beginning...
//	xSemaphoreGive(InputCaptureSem);

	vTaskSuspend(INPUTCAPTUREhandler);
	for (;;)
	{

	  
		char str[150];
		strcpy(str, "/nEntered INPUTCAPTURE Task\n Abount to ACQUIRE the Semaphore \n");

		strcat(str,"\n\n");
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		

		
	
		strcpy(str, "Leaving INPUTCAPTURE Task\n \n");
	
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);


	
    vTaskSuspend(INPUTCAPTUREhandler);

		
			
	}
}


void ADC_Task(void *pvParameters)
{

	xSemaphoreGive(ADCSem);
	while (1)
	{
		xSemaphoreTake(ADCSem, portMAX_DELAY);
		
		char str[150];
		strcpy(str, "Entered ADC Task\n Abount to ACQUIRE the Semaphore\n\n");
	
		strcat(str,"\n\n");
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		

		
	
		strcpy(str, "Leaving ADC Task\n \n\n");
		
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		
	
		
		xSemaphoreGive(ADCSem);
		vTaskSuspend(ADChandler);

		
 

	}
}

void MODBUS_Task(void *pvParameters)
{
	
	xSemaphoreGive(ModBusSem);
  
	
	while (1)
	{ 
		
		xSemaphoreTake(ModBusSem, portMAX_DELAY);

		vTaskResume(LWIPhandler);
		
		
	
 
		char str[150];
		strcpy(str, "Entered MODBUS Task\n Abount to ACQUIRE the Semaphore\n\n");
		
		strcat(str,"\n\n");
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		

		
	
		strcpy(str, "Leaving MODBUS Task\n \n\n");
	
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		
		
	
	
	xSemaphoreGive(ModBusSem);
	vTaskSuspend(LWIPhandler);
	vTaskSuspend(MODBUShandler);
	//vTaskDelay(2100);
	
	}
}

void MQTT_Task(void *pvParameters)
{
	
	
	
	while (1)
	{
		
	
		xSemaphoreTake(ADCSem, portMAX_DELAY);
		xSemaphoreTake(ModBusSem, portMAX_DELAY);
	  
		vTaskResume(LWIPhandler);
	  
   
		char str[150];
		strcpy(str, "Entered MQTT Task\n Abount to ACQUIRE the Semaphore\n\n");
		
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		
	
		
		
		strcpy(str, "Leaving MQTT Task\n \n\n");
		
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		
		
		check =2;
		vTaskDelay(500);
		
		xSemaphoreGive(ModBusSem);
		xSemaphoreGive(ADCSem); 	
		
    vTaskSuspend(LWIPhandler);		
	
   	vTaskDelay(5000);
		vTaskResume(ADChandler);
		vTaskResume(MODBUShandler);
	
			
	}
}

void LWIP_Task(void *pvParameters)
{
	char sresource[3];
	int semcount =0;
	char ssemcount[2];
  vTaskSuspend(LWIPhandler);
	while (1)
	{

		char str[150];
		
		if(check==0){
		strcpy(str, "Entered LWIP Task\n Abount to ACQUIRE the Semaphore\n\n");
		strcat(str," Setting uRTU IP\n\n");
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		check=1;
		
		}
	  
		vTaskDelay(500);
		
	
		if(check==2){
		strcpy(str, "Send Data \n Leaving LWIP Task\n\n ");
		HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		check = 1;
		
		}
		

	}
}

void displayVOL(uint32_t input_vol)
{
	char vin[] = "Vin = ";
	double volin = input_vol/pow(2,12.0f)*3.3 ;
	char newline[] = "\n\r";
	char Volin[sizeof(unsigned int)*8+1] ;
	sprintf(Volin,"%.2f",volin);
	
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) vin,strlen(vin),1000);	
		
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) Volin,strlen(Volin),1000);
	
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) newline,strlen(newline),1000);
	
}

void displayHEX(uint32_t adc_input)
{
	char hex[sizeof(unsigned int)*8+1];
	char ox[] = "0x";
	char o = '0';
	
	sprintf(hex, "%x ", adc_input);
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) ox,strlen(ox),1000);
	for(int i =1;i<=8-strlen(hex);i++)
	{	
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) &o,1,1000); 
	}
	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){};
	HAL_UART_Transmit(&huart3,(uint8_t*) hex,strlen(hex),1000);	
	
}


// UART CALLBACK FUNCTION

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		HAL_UART_Receive_IT(&huart3, &rx_data, 1);
		if(rx_data == 'r'){
			
			BaseType_t InputCaptureResume;
		
			InputCaptureResume = xTaskResumeFromISR(INPUTCAPTUREhandler);
			
			portYIELD_FROM_ISR(InputCaptureResume);
			
			
		// release the semaohore here 
			/* The xHigherPriorityTaskWokn parameter must be initialized to pdFALSE as
			it will get set to pdTRUE inside the interrupt safe API function if a 
			context switch is required.	*/
			///BaseType_t xHigherPriorityTaskWokn = pdFALSE;
			
			
			///xSemaphoreGiveFromISR(InputCaptureSem, &xHigherPriorityTaskWokn); // ISR SAFE VERSION
		//	xSemaphoreGiveFromISR(CountingSem, &xHigherPriorityTaskWokn); // ISR SAFE VERSION
		//	xSemaphoreGiveFromISR(CountingSem, &xHigherPriorityTaskWokn); // ISR SAFE VERSION
			
			
			/* Pass the xHigherPriorityTaskWokn value into portEND_SWITCHING_ISR(). If 
			xHigherPriorityTaskWokn was set to pdTRUE inside xSemaphoreGiveFromISR()
			then calling portEND_SWITCHING_ISR() will request a context switch. If
			xHigherPriorityTaskWokn is still pdFALSE then calling
			portEND_WEITCHING_ISR() will have no effect */
			
		///	portEND_SWITCHING_ISR(xHigherPriorityTaskWokn);
		}
	
	
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint32_t adcVal;
	if(hadc ->Instance==ADC1){
	
		adcVal = HAL_ADC_GetValue(hadc);
		
		adcVol = adcVal /pow(2,12.0f)*3.3;
		
}
	
}

/* USER CODE END 4 */

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

  /* USER CODE END Error_Handler_Debug */
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
