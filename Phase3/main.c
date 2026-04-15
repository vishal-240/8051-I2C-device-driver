#include <at89c5131.h>
#include "lcd.h"
#include "i2c.h"
#include "serial.h"

unsigned int dig_T1;
int dig_T2, dig_T3;

int bcd_to_dec(unsigned char val) // Convert BCD to Decimal
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

unsigned char dec_to_bcd(int val)
{
    return ((val / 10) << 4) | (val % 10);
}

void uart_print_2digit(int num)
{
    transmit_char((num / 10) + '0');
    transmit_char((num % 10) + '0');
}

void lcd_print_2digit(int num)
{
    lcd_write_char((num / 10) + '0');
    lcd_write_char((num % 10) + '0');
}

int get_temp_in_C(long raw_T)
{
    long var1, var2, t_fine;
    int T;

    var1 = ((((raw_T >> 3) - ((long)dig_T1 << 1))) * ((long)dig_T2)) >> 11;

    var2 = (((((raw_T >> 4) - ((long)dig_T1)) *
              ((raw_T >> 4) - ((long)dig_T1))) >>
             12) *
            ((long)dig_T3)) >>
           14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;

    return T; // temperature × 100
}

void get_real_time(int *hh, int *mm, int *ss)
{
    char h1, h2, m1, m2, s1, s2, temp;

    transmit_string("Enter time (HH:MM:SS): ");

    h1 = receive_char();
    h2 = receive_char();
    temp = receive_char(); // skip ':'

    m1 = receive_char();
    m2 = receive_char();
    temp = receive_char(); // skip ':'

    s1 = receive_char();
    s2 = receive_char();

    transmit_string("\r\n");

    // ASCII → integer
    *hh = (h1 - '0') * 10 + (h2 - '0');
    *mm = (m1 - '0') * 10 + (m2 - '0');
    *ss = (s1 - '0') * 10 + (s2 - '0');
}

void set_rtc_time(int hh, int mm, int ss)
{
    unsigned char bcd_sec, bcd_min, bcd_hour;

    bcd_sec  = dec_to_bcd(ss) & 0x7F; // clear CH bit
    bcd_min  = dec_to_bcd(mm);
    bcd_hour = dec_to_bcd(hh);

    I2C_WriteByte(0x68, 0x00, bcd_sec);
    I2C_WriteByte(0x68, 0x01, bcd_min);
    I2C_WriteByte(0x68, 0x02, bcd_hour);
}

void main()
{
    unsigned char sec_bcd, min_bcd, hour_bcd;
    int sec, min, hour;
    unsigned char msb, lsb, xlsb;
    long raw_T;
    int temp_C;
		int hh, mm, ss;

    lcd_init();
    uart_init();

    SSCON = 0x44; // Enable I2C
    msdelay(200); // RTC power-up delay

    I2C_WriteByte(0x76, 0xF4, 0x27); // waking up the sensor
    msdelay(100);

    dig_T1 = (I2C_ReadByte(0x76, 0x89) << 8) | I2C_ReadByte(0x76, 0x88);
    dig_T2 = (I2C_ReadByte(0x76, 0x8B) << 8) | I2C_ReadByte(0x76, 0x8A);
    dig_T3 = (I2C_ReadByte(0x76, 0x8D) << 8) | I2C_ReadByte(0x76, 0x8C);
	
		// Get time from user
		get_real_time(&hh, &mm, &ss);
		
		// Set RTC
		set_rtc_time(hh, mm, ss);

    //I2C_WriteByte(0x68, 0x00, 0x00); // Clears CH bit to start RTC

    while (1)
    {
        // Read raw temperature data from BMP280
        msb = I2C_ReadByte(0x76, 0xFA);
        lsb = I2C_ReadByte(0x76, 0xFB);
        xlsb = I2C_ReadByte(0x76, 0xFC);

        raw_T = ((long)msb << 12) | ((long)lsb << 4) | (xlsb >> 4);
        temp_C = get_temp_in_C(raw_T);

        // Read time data from RTC
        sec_bcd = I2C_ReadByte(0x68, 0x00);
        min_bcd = I2C_ReadByte(0x68, 0x01);
        hour_bcd = I2C_ReadByte(0x68, 0x02);

        // Mask unwanted bits
        sec_bcd &= 0x7F;  // clear CH bit
        hour_bcd &= 0x3F; // 24-hour mode safe mask

        // Convert BCD to Decimal
        sec = bcd_to_dec(sec_bcd);
        min = bcd_to_dec(min_bcd);
        hour = bcd_to_dec(hour_bcd);

        // Display time on LCD
        lcd_cmd(0x80);
        lcd_write_string("Time: ");
        lcd_print_2digit(hour);
        lcd_write_char(':');
        lcd_print_2digit(min);
        lcd_write_char(':');
        lcd_print_2digit(sec);

        lcd_cmd(0xC0);
        lcd_write_string("Temp: ");
        lcd_print_2digit(temp_C / 100); // Integer part
        lcd_write_char('.');
        lcd_print_2digit(temp_C % 100); // Fractional part
        lcd_write_string(" C");

        // Display time on PC via UART
        transmit_string("Time: ");
        uart_print_2digit(hour);
        transmit_char(':');
        uart_print_2digit(min);
        transmit_char(':');
        uart_print_2digit(sec);
        transmit_string("\r\n");

        // Display temperature on PC via UART
        transmit_string("Temp: ");
        uart_print_2digit(temp_C / 100);
        transmit_char('.');
        uart_print_2digit(temp_C % 100);
        transmit_string(" C\r\n");

        msdelay(1000); // Updates every second
    }
}