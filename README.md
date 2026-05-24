# Tri-axle_Balancer
Uses 3 adxl345 to detect and self correct positioning on 3 axles.

--------------Configuration----------------------

--------------------SPI Bus------------------------
hspi2 on STM32 (MASTER), hspi on ESP32 (SLAVE), SPI on ADXL345 (SLAVE)
- SCLK: STM32 PB10, ADXL SCL, ESP32 HSPI CLK (GPIO14)
- MISO: STM32 PC2, ADXL SDO, ESP32 HSPI MISO (GPIO12)
- MOSI: STM32 PC3, ADXL SDA, ES32 HSPI MOSI (GPIO13)
- CS/SS: STM32 PB12 -> ADXL CS, STM32 PB13 -> ESP32 HSPI CS0 (GPIO15)
------------------End SPI Bus-----------------------
  
- PA12: LED_OUT (for testing)
- CPOL: HIGH (1)
- CPHA: 2 Edge (1)
- MSB First

- PA7-PA10: PWM to stepper for correction
- PA2, PA3: UART TX,RX, default settings: relaying current angle and corrections to terminal
- PA1: Receive interrupt signal from accelerometer


--------Serial Communication Flowchart------------------
Read_Voltage() - ADXL345 SDO to STM32 MISO;
               - Blank message sent from MOSI to SDA
               - HAL_SPI_TransmitReceive_IT(&hspi2, TX_Buffer, RX_Buffer, 7) ---[7 bytes sent and received]

Angle_Show()   - UART TX to Serial window on cubeIDE
               - HAL_UART_TransmitReceive_IT(&huart2, UART_TX_Buf, UART_RX_Buf, )

