#ifndef __STM32F10x_LCD_H
#define __STM32F10x_LCD_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void delay_nus(unsigned long n);
void delay_nms(unsigned long n);
void LCD_Write(BitAction isData,u8 Data);
void LCD_SetCursor(unsigned char x, unsigned char y);
void LCD_DisplayString(unsigned char x, unsigned char y, unsigned char *s);
void LCD_DisplayOneLine(unsigned char row, unsigned char *s);
void LCD_DisplayTwoLine(unsigned char row, unsigned char *s);
void LCD_Clear(void);
void LCD_Init(void);

#endif
