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

- PA10: PWM to servo for correction
- PA2, PA3: USARTTX,RX, default settings: relaying current angle and corrections to terminal
- PA1: Receive interrupt signal from accelerometer
