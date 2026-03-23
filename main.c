#include "main.h"
#include "cmsis_os.h"
#include <math.h>

#define		ADXL_READ		0x80
#define		ADXL_MULTI		0x40
#define 	ADXL_DATAX0		0x32
#define		FLAG_SPI_DONE	(1U << 0)
#define		FLAG_X_DONE		(1U << 1)
#define		FLAG_Y_DONE		(1U << 2)
#define		FLAG_Z_DONE		(1U << 3)
#define		FLAG_UART_DONE	(1U << 4)

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart2;

float referenceAngles[3];
float currentAngles[3];

typedef struct{
	float ax;
	float ay;
	float az;
	float angleX;
	float angleY;
	float angleZ;
}AngleMsg_t;

typedef struct{
	int16_t xRaw;			//Signed twos complement values from accelerometer
	int16_t yRaw;
	int16_t zRaw;
} RawImu_t;

uint8_t TX_Buffer[7];
uint8_t RX_Buffer[7]; //Initialize receive buffer for spi comms

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Read_Angle_Volt */
osThreadId_t Read_Angle_VoltHandle;
const osThreadAttr_t Read_Angle_Volt_attributes = {
  .name = "Read_Angle_Volt",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


/* Definitions for Angle_Convert */
osThreadId_t Angle_ConvertHandle;
const osThreadAttr_t Angle_Convert_attributes = {
  .name = "Angle_Convert",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED_Out */
osThreadId_t LED_OutHandle;
const osThreadAttr_t LED_Out_attributes = {
  .name = "LED_Out",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Print_Angle */
osThreadId_t Print_AngleHandle;
const osThreadAttr_t Print_Angle_attributes = {
  .name = "Print_Angle",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Correct_angle */
osThreadId_t Correct_angleHandle;
const osThreadAttr_t Correct_angle_attributes = {
  .name = "Correct_angle",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for qRawImu */
osMessageQueueId_t qRawImuHandle;
const osMessageQueueAttr_t qRawImu_attributes = {
  .name = "qRawImu"
};
/* Definitions for qAngles */
osMessageQueueId_t qAnglesHandle;
const osMessageQueueAttr_t qAngles_attributes = {
  .name = "qAngles"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
void initMotor(float baseX, float baseY, float baseZ);
void StartDefaultTask(void *argument);
void Read_Angle(void *argument);
void Angle_Conversion(void *argument);
void BlinkLED(void *argument);
void Angle_Show(char *UART_TX_Buf);
void Angle_Correct(void *argument);
void PWM_Step_1(void);
void PWM_Step_2(void);
void PWM_Step_3(void);
void PWM_Step_4(void);
void CW_Rotation(void);
void CCW_Rotation(void);
void Correct_X(AngleMsg_t *ang);
void Correct_Y(AngleMsg_t *ang);
void Correct_Z(AngleMsg_t *ang);


/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  SystemClock_Config();

  /* Initialize all configured peripherals and motor*/
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /*INSERT MOTOR BASELINE ANGLES BEFORE RUNNING*/
  initMotor(0.0f, 0.0f, 0.0f);

  /* Init scheduler */
  osKernelInitialize();

  /* Create the queue(s) */
  /* creation of qRawImu */
  qRawImuHandle = osMessageQueueNew (8, sizeof(RawImu_t), &qRawImu_attributes);

  /* creation of qAngles */
  qAnglesHandle = osMessageQueueNew (4, sizeof(AngleMsg_t), &qAngles_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Read_Angle_Volt */
  Read_Angle_VoltHandle = osThreadNew(Read_Angle, NULL, &Read_Angle_Volt_attributes);

  /* creation of Angle_Convert */
  Angle_ConvertHandle = osThreadNew(Angle_Conversion, NULL, &Angle_Convert_attributes);

  /* creation of LED_Out */
  LED_OutHandle = osThreadNew(BlinkLED, NULL, &LED_Out_attributes);

  /* creation of Print_Angle */
  Print_AngleHandle = osThreadNew(Angle_Show, NULL, &Print_Angle_attributes);


  /* creation of Correct_angle, rotates stepper motor to correct position */
  Correct_angleHandle = osThreadNew(Angle_Correct, NULL, &Correct_angle_attributes);


  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|PWM_Motor_Pin|PA_7_Pin|PA_8_Pin|PA_9_Pin|LED_out_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Interrupt_Pin */
  GPIO_InitStruct.Pin = Interrupt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Interrupt_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin PWM pins LED_out_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|PWM_Motor_Pin|LED_out_Pin|PA_7_Pin|PA_8_Pin|PA_9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/*Set initial accelerometer angle baseline values*/
void initMotor(float baseX, float baseY, float baseZ){
	referenceAngles[0] = baseX;
	referenceAngles[1] = baseY;
	referenceAngles[2] = baseZ;
}

//PWM Step functions for motor rotation
void PWM_Step_1(void){
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_SET);
	osDelay(1);
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_RESET);
}
void PWM_Step_2(void){
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PA_8_Pin, GPIO_PIN_SET);
	osDelay(1);
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PA_8_Pin, GPIO_PIN_RESET);
}
void PWM_Step_3(void){
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_SET);
	osDelay(1);
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_RESET);
}
void PWM_Step_4(void){
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_SET);
	osDelay(1);
	HAL_GPIO_WritePin(GPIOA, PA_7_Pin|PWM_Motor_Pin, GPIO_PIN_RESET);
}

void CW_Rotation(void){
	PWM_Step_1();
	PWM_Step_2();
	PWM_Step_3();
	PWM_Step_4();
}

void CCW_Rotation(void){
	PWM_Step_4();
	PWM_Step_3();
	PWM_Step_2();
	PWM_Step_1();
}

void Correct_X(AngleMsg_t *ang){
	float error = referenceAngles[0] - ang->angleX;		//Sets tilt error value
	if(fabsf(error) <= 2.5f){		//Doesn't correct if error is less than +/- 2.5 degrees
		return;
	}
	if(error > 0){
		CW_Rotation;			//Rotates clockwise if error is positive
	}
	else{
		CCW_Rotation;		//Rotates ccw is error is negative (rotation directions based off unit circle)
	}
	osThreadFlagsSet(Angle_ConvertHandle, FLAG_X_DONE);
}
void Correct_Y(AngleMsg_t *ang){
	float error = referenceAngles[1] - ang->angleX;		//Sets tilt error value
	if(fabsf(error) <= 2.5f){		//Doesn't correct if error is less than +/- 2.5 degree
		return;
	}
	if(error > 0){
		CW_Rotation;			//Rotates clockwise if error is positive
	}
	else{
		CCW_Rotation;		//Rotates ccw is error is negative (rotation directions based off unit circle)
	}
	osThreadFlagsSet(Angle_ConvertHandle, FLAG_Y_DONE);
}
void Correct_Z(AngleMsg_t *ang){
	float error = referenceAngles[2] - ang->angleZ;
	if(fabsf(error) <= 1.0f){
		return;
	}
	if(error > 0){
		CW_Rotation;
	}
	else{
		CCW_Rotation;
	}
	osThreadFlagsSet(Angle_ConvertHandle, FLAG_Z_DONE);
}



void StartDefaultTask(void *argument) //Initialize baseline angles
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

void Read_Angle(void *argument)
{
  /* USER CODE BEGIN Read_Angle */
  RawImu_t raw;
  for(;;)
  {
	TX_Buffer[0] = ADXL_READ | ADXL_MULTI | ADXL_DATAX0; //Sets Transmission buffer with SPI command byte
    for(int i = 1; i < 7; i++){
    	TX_Buffer[i] = 0x00;							//Replaces all data in transmission buffer with 0
    }
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //CS Low, enables ADXL to start transmitting data

    HAL_SPI_TransmitReceive_IT(&hspi2, TX_Buffer, RX_Buffer, 7);		//SPI_2 pointer, tx buffer, rx buffer, amount of data being exchanged

	osThreadFlagsWait(FLAG_SPI_DONE, osFlagsWaitAny, osWaitForever); /*Wait until transmission done. Transmission stopped in callback
	HAL_SPI_TxRxCpltCallback */

	//Move X, Y, and Z counts into variables. 16-bits for each axis
	raw.xRaw = (int16_t)((RX_Buffer[2] << 8) | RX_Buffer[1]);
	raw.yRaw = (int16_t)((RX_Buffer[4] << 8) | RX_Buffer[3]);
	raw.zRaw = (int16_t)((RX_Buffer[6] << 8) | RX_Buffer[5]);

	//Push to queue
	osMessageQueuePut(qRawImuHandle, &raw, 0, 0);
  }
  /* USER CODE END Read_Angle */
}


void Angle_Conversion(void *argument)
{
  /* USER CODE BEGIN Angle_Conversion */

	RawImu_t raw;
	AngleMsg_t ang;

  /* Infinite loop */
  for(;;)
  {
	  /*Get Voltage LSBs from RawImu queue*/
	  osMessageQueueGet(qRawImuHandle, &raw, NULL, osWaitForever);

	  /*Accelerometer gives digital counts per g, each LSB is 0.004g*/
	  ang.ax = raw.xRaw * 0.004f;   // acceleration in g
	  ang.ay = raw.yRaw * 0.004f;
	  ang.az = raw.zRaw * 0.004f;

	  ang.angleX = atan2(ang.ax, sqrt(ang.ay*ang.ay + ang.az*ang.az)) * 180.0 / M_PI;     // X-axis
	  ang.angleY = atan2(ang.ay, sqrt(ang.ax*ang.ax + ang.az*ang.az)) * 180.0 / M_PI;     // Y-axis
	  ang.angleZ = atan2(sqrt(ang.ax*ang.ax + ang.ay*ang.ay), ang.az) * 180.0 / M_PI;     // Z-axis

	  /*Push angles onto Angles queue*/
	  osMessageQueuePut(qAnglesHandle, &ang, 0, 0);
  }
  /* USER CODE END Angle_Conversion */
}

/* USER CODE BEGIN Header_BlinkLED */
/**
* @brief Function implementing the LED_Out thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BlinkLED */
void BlinkLED(void *argument)
{
  /* USER CODE BEGIN BlinkLED */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END BlinkLED */
}

/* USER CODE BEGIN Header_Angle_Show */
/**
* @brief Function implementing the Print_Angle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Angle_Show */
//Send angle values to uart terminal
void Angle_Show(void *argument)
{
  /* USER CODE BEGIN Angle_Show */
	AngleMsg_t ang;
  /* Infinite loop */
  for(;;)
  {
	  osMessageQueueGet(qAnglesHandle, &ang, NULL, osWaitForever);
	  
	  HAL_UART_Transmit_IT(&huart2, UART_TX_Buf, 7);
	  osDelay(1);
  }
  /* USER CODE END Angle_Show */
}

void Angle_Correct(void *argument)
{
  /* USER CODE BEGIN Angle_Correct */
	AngleMsg_t ang;
  /* Infinite loop */
  for(;;)
  {
	  osMessageQueueGet(qAnglesHandle, &ang, NULL, osWaitForever);
	  /*Motor is 28-BYJ84 Stepper motor, rotates 5.625 degrees per step,
	   * requires 64 steps to perform a 360 degree rotation*/

	  /*X axis (1 motor per axis)*/
	  Correct_X(&ang);
	  /*Y axis*/
	  Correct_Y(&ang);
	  /*Z axis*/
	  Correct_Z(&ang);
  }
  /* USER CODE END Angle_Correct */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM5 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM5)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
	if(hspi->Instance == SPI2){
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET); //CS HIGH
		osThreadFlagsSet(Read_Angle_VoltHandle, FLAG_SPI_DONE); //Wake task
	}
}
void HAL_UART_TxCpltCallback(SPI_HandleTypeDef *huart2){
	if(hspi->Instance == UART2){
		osThreadFlagsSet(Read_Angle_VoltHandle, FLAG_UART_DONE); //Wake task
	}
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
