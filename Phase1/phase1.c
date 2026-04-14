#include <at89c5131.h>
#include "lcd.h"

// Control bits
#define SI  0x08 //bit 3: SI flag
#define STA 0x20 //bit 5: Start condition
#define STO 0x10 //bit 4: Stop condition
#define AA  0x04 //bit 2: Acknowledge enable

#define BMP280_ADDR 0x76 // I2C address of BMP280 if SDO is gnd

void lcd_print_hex(unsigned char val)
{
    unsigned char high, low;

    high = (val >> 4) & 0x0F;
    low = val & 0x0F;

    high += (high < 10) ? '0' : 'A' - 10;
    low += (low < 10) ? '0' : 'A' - 10;

    lcd_write_char(high);
    lcd_write_char(low);
}

void I2C_Wait() {
    while(!(SSCON & SI));   // wait for completion
}

void I2C_Check(unsigned char expected) { //Checking the status code against expected value
    unsigned char status = SSCS;

    if(status != expected) {
        lcd_cmd(0x01);
        lcd_write_string("ERR:");
        lcd_print_hex(status);
        while(1);
    }
}

void I2C_Start(unsigned char expected_status) {
    SSCON |= STA;
    SSCON &= ~SI;

    I2C_Wait();
    I2C_Check(expected_status);

    SSCON &= ~STA;
}

void I2C_Stop() {
    SSCON |= STO;
    SSCON &= ~SI;
		
	while(SSCON & STO); //since the STOP condition is automatically cleared
}

void I2C_Write(unsigned char tx_data, unsigned char expected_status) {
    SSDAT = tx_data;
    SSCON &= ~SI;

    I2C_Wait();
    I2C_Check(expected_status);
}

unsigned char I2C_Read(unsigned char ack, unsigned char expected_status) {
    if(ack)
        SSCON |= AA;
    else
        SSCON &= ~AA;

    SSCON &= ~SI;

    I2C_Wait();
    I2C_Check(expected_status);

    return SSDAT;
}

unsigned char BMP280_ReadID() {
    unsigned char id;

    I2C_Start(0x08); // start 

    // SLAVE Addr + WRITE → expect ACK (0x18)
    I2C_Write((BMP280_ADDR << 1) | 0, 0x18);

    // Register address → expect ACK (0x28)
    I2C_Write(0xD0, 0x28);

    I2C_Start(0x10); // repeated start

    // SLAVE Addr + READ → expect ACK (0x40)
    I2C_Write((BMP280_ADDR << 1) | 1, 0x40);

    // READ DATA → expect (0x58 = data received, NACK returned)
    id = I2C_Read(0, 0x58);

    I2C_Stop();

    return id;
}

void main()
{
    unsigned char chip_id;

    lcd_init();

    // Enable TWI
    SSCON = 0x44;

    msdelay(100);

    chip_id = BMP280_ReadID();

    lcd_cmd(0x80);
    lcd_write_string("BMP280 ID:");

    lcd_cmd(0xC0);
    lcd_write_string("0x");
    lcd_print_hex(chip_id);

    while (1);
}