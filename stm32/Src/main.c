#include "main.h"

I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
uint8_t Read_AD7768_Register(uint8_t address);
void Write_AD7768_Register(uint8_t address, uint8_t value);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();

  /*SET Si514 on I2C1 to the correct frequency*/
  //uint8_t XORegisters[8] = {0x22, 0xDF, 0x1F, 0x59, 0x25, 0x08, 0x50, 0x23}; //614.4 kHz
  //uint8_t Registers[8] = {0x23, 0x73, 0xD0, 0x96, 0x2E, 0x26, 0x64, 0x01}; //7.12 MHz
  uint8_t XORegisters[8] = {0x22, 0x6F, 0xB8, 0x80, 0x24, 0x08, 0x64, 0x33}; //300 kHz
  
  uint8_t buf[8];
  //Deassert OE bit
  buf[0] = 0x84; //Register Address
  buf[1] = 0x00; //Register Value
  HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, buf, 2, 50);

  //Write the new frequency configuration (LP1 and LP2)
  buf[0] = 0x00;
  buf[1] = XORegisters[0];
  HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, buf, 2, 50);

  //Write the new frequency configuration (M, HS_DIV, HS_DIV)
  buf[0] = 0x05;
  for(int i = 1; i < 8; i ++){buf[i] = XORegisters[i];}
  HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, buf, 8, 50);
  
  //Assert FCAL register bit (This bit is self-clearing) and assert OE register bit
  buf[0] = 0x84;
  buf[1] = 0x05;
  HAL_I2C_Master_Transmit(&hi2c1, 0x55 << 1, buf, 2, 50);

  uint8_t counter = 0;
  while (1)
  {

    HAL_GPIO_TogglePin(GPIOA, MODE0_Pin);

    //Write_AD7768_Register(0x15, 0b00000000);//set ADC to 16bit mode

    uint8_t address = 0x2C;
    uint8_t tx_buf[4];
    uint8_t rx_buf[4];
    tx_buf[0] = 0x00; //second byte to be transmitted, irrelevant in case of write
    tx_buf[1] = (1 << 6) | (address & ~(0b11 << 6)); //first byte to be transmitted, address is expected to be right-aligned
    tx_buf[2] = 0x00;
    tx_buf[3] = 0x00;
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 2, 50);
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_SET);

    //Scratchpad Echo Example
    /*Write_AD7768_Register(0x0A, counter);
    counter ++;
    uint8_t scratchpad = Read_AD7768_Register(0x0A);
    HAL_UART_Transmit(&huart2, &scratchpad, 1, 50);*/

    HAL_Delay(100);
  }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /**Initializes the CPU, AHB and APB busses clocks 
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
  /**Initializes the CPU, AHB and APB busses clocks 
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

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
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


  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MODE0_GPIO_Port, MODE0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MODE1_Pin|MODE3_Pin|MODE2_Pin|CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MCLK_Pin */
  GPIO_InitStruct.Pin = MCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MODE0_Pin */
  GPIO_InitStruct.Pin = MODE0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MODE0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MODE1_Pin MODE3_Pin MODE2_Pin CS_Pin */
  GPIO_InitStruct.Pin = MODE1_Pin|MODE3_Pin|MODE2_Pin|CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void Error_Handler() {

}

uint8_t Read_AD7768_Register(uint8_t address){
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    tx_buf[0] = 0x00; //second byte to be transmitted, irrelevant in case of write
    tx_buf[1] = (1 << 6) | (address & ~(0b11 << 6)); //first byte to be transmitted, address is expected to be right-aligned
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 1, 50);
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_SET);
    return rx_buf[0];
}

void Write_AD7768_Register(uint8_t address, uint8_t value){
    uint8_t tx_buf[2];
    tx_buf[0] = value; //second byte to be transmitted, irrelevant in case of write
    tx_buf[1] = (0 << 6) | (address & ~(0b11 << 6)); //first byte to be transmitted, address is expected to be right-aligned
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, tx_buf, 1, 50);
    HAL_GPIO_WritePin(GPIOB, CS_Pin, GPIO_PIN_SET);
}
