# Tri-axle_Balancer
Uses 3 adxl345 to detect and self correct positioning on 3 axles.

--------------Configuration----------------------
SPI2 on STM32f401RET6
- PB10: SCK
- PC2: MISO
- PC3: MOSI
- PB12: CS
- PA12: LED_OUT (for testing)
- CPOL: HIGH (1)
- CPHA: 2 Edge (1)
- MSB First

- PA7-PA10: PWM to servo for correction
- PA2, PA3: UART TX,RX, default settings: relaying current angle and corrections to terminal
- PA1: Receive interrupt signal from accelerometer


--------Serial Communication Flowchart------------------
Read_Voltage() - ADXL345 SDO to STM32 MISO;
               - Blank message sent from MOSI to SDA
               - HAL_SPI_TransmitReceive_IT(&hspi2, TX_Buffer, RX_Buffer, 7) ---[7 bytes sent and received]

Angle_Show()   - UART TX to Serial window on cubeIDE
               - HAL_UART_TransmitReceive_IT(&huart2, UART_TX_Buf, UART_RX_Buf, )

