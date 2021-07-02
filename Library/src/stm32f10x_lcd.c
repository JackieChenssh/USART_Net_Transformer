#include "stm32f10x_lcd.h"

void delay_nus(unsigned long n)
{
    unsigned long j;
    while(n--)
    {
        j=8;
        while(j--);
    }
}

void delay_nms(unsigned long n)
{
    while(n--)
        delay_nus(1100);
}

void LCD_Write(BitAction isData,u8 Data)
{
    GPIO_ResetBits(GPIOD,GPIO_Pin_5);
    GPIO_WriteBit(GPIOD,GPIO_Pin_4,isData);
    GPIO_Write(GPIOC, Data);
    GPIO_ResetBits(GPIOD,GPIO_Pin_6);
    delay_nms(5);
    GPIO_SetBits(GPIOD,GPIO_Pin_6);
    delay_nms(5);
    GPIO_ResetBits(GPIOD,GPIO_Pin_6);
}

void LCD_SetCursor(unsigned char x, unsigned char y)
{
    if(x <= 1) 
        LCD_Write(0,((0x80 + y) | (x << 6)));
}

void LCD_DisplayString(unsigned char x, unsigned char y, unsigned char *s)
{
    LCD_SetCursor(x,y);
    while(*s) 
        LCD_Write(1,*(s++));
}

void LCD_DisplayOneLine(unsigned char row, unsigned char *s)
{
    char Max_Word = 16;
    LCD_SetCursor(row,0);
    while(*s && Max_Word--) 
        LCD_Write(1,*(s++));
}

void LCD_DisplayTwoLine(unsigned char row, unsigned char *s)
{
    char Max_Word = 16;
    LCD_SetCursor(row,0);
    while(*s && Max_Word--) 
        LCD_Write(1,*(s++));
	Max_Word = 16;
	if(row == 1)
		LCD_SetCursor(0,0);
	else
		LCD_SetCursor(1,0); 
	while(*s && Max_Word--) 
        LCD_Write(1,*(s++));
}


void LCD_Clear(void)
{
    LCD_Write(0,0x38);
    LCD_Write(0,0x08);
    LCD_Write(0,0x01);
    LCD_Write(0,0x06);
    LCD_Write(0,0x0C);
}

void LCD_Init(void)
{
	extern GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	LCD_Clear();
}
