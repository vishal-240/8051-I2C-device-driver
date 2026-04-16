# 8051-I2C-device-driver
Bare-metal I2C driver development on AT89C5131 (8051) with BMP280 sensor and DS1307 RTC integration, featuring LCD debugging and UART communication.   

**EE337 course project**. 

It was done in 3 phases/weeks:

**Phase 1** - Bare metal code without bit banging for accessing/reading the WhoamI register of the BMP280 sensor.

**Phase 2** - Writing the code in a I2C driver library format for easy access. 

**Phase 3** - Using my own I2C driver library to interface an RTC(Real-Time Clock) module,DS1307, with the 8051 microcontroller. We use it to print the time and temperature data from BMP280 onto the LCD and also the PC using UART.
