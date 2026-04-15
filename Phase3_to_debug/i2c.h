#ifndef I2C_H
#define I2C_H

void I2C_Start(void);

void I2C_Stop(void);

void I2C_Write(unsigned char write_data);

unsigned char I2C_Read(void);

void I2C_Ack(void);

void I2C_Nack(void);

unsigned char I2C_ReadByte(unsigned char device_addr, unsigned char reg_addr);

void I2C_WriteByte(unsigned char device_addr, unsigned char reg_addr, unsigned char write_data);

#endif