#include <at89c5131.h>
#include "lcd.h"
#include "i2c.h"
#include "serial.h"

// ---------- DEBUG PRINT ----------
void debug_msg(char *msg)
{
    lcd_cmd(0x01);          // clear screen
    lcd_write_string(msg);
    msdelay(1000);
}

// ---------- PRINT HEX ----------
void lcd_print_hex(unsigned char val)
{
    unsigned char high = (val >> 4) & 0x0F;
    unsigned char low  = val & 0x0F;

    high += (high < 10) ? '0' : 'A' - 10;
    low  += (low  < 10) ? '0' : 'A' - 10;

    lcd_write_char(high);
    lcd_write_char(low);
}

// ---------- SHOW STATUS ----------
void show_status()
{
    lcd_cmd(0x01);
    lcd_write_string("SSCS:");
    lcd_print_hex(SSCS);
    while(1);   // freeze here
}

// ---------- SAFE READ ----------
unsigned char safe_read(unsigned char dev, unsigned char reg)
{
    unsigned char val;

    I2C_Start();

    I2C_Write((dev << 1) | 0);
    if(SSCS != 0x18) show_status();

    I2C_Write(reg);
    if(SSCS != 0x28) show_status();

    I2C_Start();
    if(SSCS != 0x10) show_status();

    I2C_Write((dev << 1) | 1);
    if(SSCS != 0x40) show_status();

    I2C_Nack();
    val = I2C_Read();

    if(SSCS != 0x58) show_status();

    I2C_Stop();

    return val;
}

// ---------- MAIN ----------
void main()
{
    unsigned char val;

    lcd_init();
    uart_init();

    debug_msg("START");

    SSCON = 0x44;
    msdelay(200);

    debug_msg("I2C OK");

    // ---------- BMP TEST ----------
    debug_msg("BMP WAKE");

    I2C_WriteByte(0x76, 0xF4, 0x27);
    msdelay(100);

    debug_msg("BMP READ");

    val = safe_read(0x76, 0xD0);   // read ID

    lcd_cmd(0x01);
    lcd_write_string("BMP ID:");
    lcd_print_hex(val);
    msdelay(2000);

    // ---------- RTC TEST ----------
    debug_msg("RTC INIT");

    I2C_WriteByte(0x68, 0x00, 0x00);

    debug_msg("RTC READ");

    val = safe_read(0x68, 0x00);   // read seconds

    lcd_cmd(0x01);
    lcd_write_string("SEC:");
    lcd_print_hex(val);
    msdelay(2000);

    // ---------- LOOP ----------
    while(1)
    {
        lcd_cmd(0x01);
        lcd_write_string("RUNNING");
        msdelay(1000);
    }
}