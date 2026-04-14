#include <at89c5131.h>
#include "lcd.h"
#include "i2c.h"

unsigned int dig_T1;
int dig_T2, dig_T3;

#define BMP280_ADDR 0x76

// Fast sequential read for calibration data
void read_calibration()
{
    unsigned char lsb, msb;

    I2C_Start();
    I2C_Write((BMP280_ADDR << 1) | 0); // Write mode
    I2C_Write(0x88);                   // Start at dig_T1 LSB

    I2C_Start(); // Repeated start
    I2C_Write((BMP280_ADDR << 1) | 1); // Read mode

    I2C_Ack(); lsb = I2C_Read();
    I2C_Ack(); msb = I2C_Read();
    dig_T1 = (msb << 8) | lsb;

    I2C_Ack(); lsb = I2C_Read();
    I2C_Ack(); msb = I2C_Read();
    dig_T2 = (msb << 8) | lsb;

    I2C_Ack();  lsb = I2C_Read();
    I2C_Nack(); msb = I2C_Read(); // NACK on the final byte
    dig_T3 = (msb << 8) | lsb;

    I2C_Stop();
}

// Fast sequential read for temperature registers
long read_raw_temp()
{
    unsigned char msb, lsb, xlsb;
    long adc_T;

    I2C_Start();
    I2C_Write((BMP280_ADDR << 1) | 0); // Write mode
    I2C_Write(0xFA);                   // Start at temp MSB

    I2C_Start(); // Repeated start
    I2C_Write((BMP280_ADDR << 1) | 1); // Read mode

    I2C_Ack();  msb = I2C_Read();
    I2C_Ack();  lsb = I2C_Read();
    I2C_Nack(); xlsb = I2C_Read(); // NACK on the final byte

    I2C_Stop();

    adc_T = ((long)msb << 12) | ((long)lsb << 4) | (xlsb >> 4);
    return adc_T;
}

// Bosch Compensation Algorithm
int compensate_temp(long adc_T)
{
    long var1, var2, t_fine;
    int T;

    var1 = ((((adc_T >> 3) - ((long)dig_T1 << 1))) * ((long)dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((long)dig_T1)) *
              ((adc_T >> 4) - ((long)dig_T1))) >> 12) *
              ((long)dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;  // temp * 100
}

void main()
{
    long adc_T;
    int T;
    int temp_int, temp_frac;

    lcd_init();

    SSCON = 0x44;   // Enable I2C Hardware
    msdelay(100);

    // Wake sensor & configure (1x Oversampling, Normal Mode)
    I2C_WriteByte(BMP280_ADDR, 0xF4, 0x27);
    msdelay(100);

    // Read calibration constants once
    read_calibration();

    while(1)
    {
        adc_T = read_raw_temp();
        T = compensate_temp(adc_T);

        temp_int = T / 100;
        temp_frac = T % 100;

        lcd_cmd(0x80);
        lcd_write_string("Temp:");

        lcd_cmd(0xC0);
        lcd_print_int(temp_int);
        
        // Print fractional part safely
        lcd_write_char('.');
        if (temp_frac < 10) {
            lcd_write_char('0'); // Add leading zero if needed
        }
        lcd_print_int(temp_frac);
        lcd_write_string(" C");

        msdelay(500);
    }
}