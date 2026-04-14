#include <at89c5131.h>
#include "i2c.h"

// Control bits
#define SI  0x08 //bit 3: SI flag
#define STA 0x20 //bit 5: Start condition
#define STO 0x10 //bit 4: Stop condition
#define AA  0x04 //bit 2: Acknowledge enable

static void I2C_Wait() {
    while(!(SSCON & SI));   // wait for completion
}

void I2C_Start() {
    SSCON |= STA;
    SSCON &= ~SI;

    I2C_Wait();
    SSCON &= ~STA;
}

void I2C_Stop() {
    SSCON |= STO;
    SSCON &= ~SI;
        
    while(SSCON & STO); //since the STOP condition is automatically cleared
}

void I2C_Write(unsigned char write_data) {
    SSDAT = write_data;
    SSCON &= ~SI;

    I2C_Wait();
}

unsigned char I2C_Read() {
    SSCON &= ~SI;

    I2C_Wait();

    return SSDAT;
}

void I2C_Ack() {
    SSCON |= AA;
}

void I2C_Nack() {
    SSCON &= ~AA;
}

unsigned char I2C_ReadByte(unsigned char device_addr, unsigned char reg_addr) { //for reading just one byte from the device
    unsigned char read_data;

    I2C_Start();

    I2C_Write((device_addr << 1) | 0); // Write mode
    I2C_Write(reg_addr);

    I2C_Start(); // Repeated start

    I2C_Write((device_addr << 1) | 1); // Read mode

    I2C_Nack(); // NACK before reading the byte
    read_data = I2C_Read();

    I2C_Stop();

    return read_data;
}

void I2C_WriteByte(unsigned char device_addr, unsigned char reg_addr, unsigned char write_data) { //for writing just one byte to the device
    I2C_Start();

    I2C_Write((device_addr << 1) | 0); // Write mode
    I2C_Write(reg_addr);
    I2C_Write(write_data);

    I2C_Stop();
}

