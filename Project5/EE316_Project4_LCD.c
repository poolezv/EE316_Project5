#include "xparameters.h"   
#include "xgpio.h"         
#include "xil_printf.h"    
#include "xil_exception.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xadcps.h"
#include "xstatus.h"
#include "PWM.h"
#include "xsysmon.h"
#include "xparameters.h"
#include "sleep.h"
#include "stdio.h"
#include "xgpio.h"
#include "xil_types.h"
#include "debounce.h"

#define RS 0x01
#define EN 0x02
#define LCD_CHANNEL 1

XGpio LCD;

void LCD_nibble_write(char data, unsigned char control);
void LCD_command(unsigned char command);
void LCD_init(void);
void LCD_data(char data);
void LCD_write_line(char* cstring);
void LCD_clear_all(void); // Clear both lines
void LCD_set_line(int line); // Clear line and set to position 1


int main(){
	
	xil_printf("Initializing LCD \r\n");
	XGpio_Initialize(&LCD_inst, LCD_ID);
	LCD_init();
	LCD_write_line("Enable");
	LCD_command(0xC0);
	LCD_write_line("POT");
	
}

void LCD_init(void) {
	
    usleep(20000);                /* LCD controller reset sequence */
    LCD_nibble_write(0x30, 0);
    usleep(5000);
    LCD_nibble_write(0x30, 0);
    usleep(1000);
    LCD_nibble_write(0x30, 0);
    usleep(1000);

    LCD_nibble_write(0x20, 0);  /* use 4-bit data mode */
    usleep(1000);
    LCD_command(0x28);          /* set 4-bit data, 2-line, 5x7 font */
    LCD_command(0x06);          /* move cursor right */
    LCD_command(0x01);          /* clear screen, move cursor to home */
    LCD_command(0x0F);          /* turn on display, cursor blinking */
	
}

void LCD_nibble_write(char data, unsigned char control) {
	
	volatile uint8_t bits = 0x00;
	
	if(control & RS){
		bits |= (RS << 4);
	}else{
		bits &= ~(RS << 4);
	}
	
	bits &= ~(EN << 4);
	XGpio_DiscreteWrite(&LCD, LCD_CHANNEL, bits);
	bits |= EN << 4;
	XGpio_DiscreteWrite(&LCD, LCD_CHANNEL, bits);
	
	usleep(0);
	
	bits &= ~(EN << 4);
	XGpio_DiscreteWrite(&LCD, LCD_CHANNEL, bits);
}

void LCD_command(unsigned char command) {
    LCD_nibble_write(command & 0xF0, 0);    /* upper nibble first */
    LCD_nibble_write(command << 4, 0);      /* then lower nibble */

    if (command < 4)
        usleep(2000);         /* command 1 and 2 needs up to 1.64ms */
    else
        usleep(1000);         /* all others 40 us */
}

void LCD_data(char data) {
    LCD_nibble_write(data & 0xF0, RS);      /* upper nibble first */
    LCD_nibble_write(data << 4, RS);        /* then lower nibble */

 	usleep(1000);
}


	
	
	
	
	
	
	
	