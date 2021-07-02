#include <stdio.h>
#include "string.h"
#include "ENC28J60.h"
#include "Net_Config.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h" 
#include "stm32f10x_nvic.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_lcd.h"
#include "stm32f10x_dma.h"

#define LocalPort_NUM 8000
#define DMA_Channel5        ((DMA_Channel_TypeDef *) DMA_Channel5_BASE)

GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
NVIC_InitTypeDef NVIC_InitStruct;	
SPI_InitTypeDef SPI_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
extern void SystemInit(void);

u8 TimeFlag = 0;

u8 udp_soc;
u8 Rem_IP[4] = {0,0,0,0};
u16 R_Port = 8080;

u16 RecLen = 0;
u8 Recbuf[ETH_MTU];
u16 RecLenSum = 0;

volatile u16 USART_RecLen = 0;
u8 USART_Recbuf[ETH_MTU];
volatile u16 USART_RecLenSum = 0;

int SendChar(int ch) 
{
	while (!(USART1->SR & USART_FLAG_TXE)) ;
	USART1->DR = (ch & 0x1FF);
	return ch;
}

int GetKey(void) 
{
	while (!(USART1->SR & USART_FLAG_RXNE)) ;
	return ((int) (USART1->DR & 0x1FF));
}

void LED_GPIO_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void SPI2_Port_Configuration(void)
{
	/* Configure SPI2 pins: NSS, SCK, MISO and MOSI */ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
}

void SPI2_Configuration(void)
{
	/* SPI2 configuration */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	SPI_InitStructure.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	/* Enable SPI2	*/ 
	SPI_Cmd(SPI2, ENABLE);
}

void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQChannel;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Period = 7199;
	TIM_TimeBaseStructure.TIM_Prescaler = 999;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update); 
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void USART1_Init(void)
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA, ENABLE);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQChannel;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ClearFlag(USART1, USART_FLAG_TC | USART_FLAG_IDLE);
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
	USART_Cmd(USART1, ENABLE);

	DMA_DeInit(DMA_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;				
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART_Recbuf; 		
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	
	DMA_InitStructure.DMA_BufferSize = ETH_MTU;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 	
	DMA_Init(DMA_Channel5,&DMA_InitStructure);  
	DMA_Cmd(DMA_Channel5,ENABLE);
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);

}

void UDP_SendData(u8 *dat,u16 len,u8 *ip,u16 Port) 
{
	u8 *sendbuf;
	u8 res = 0;
	sendbuf = udp_get_buf(len);
	if(sendbuf != NULL) 
	{
		memcpy(sendbuf,dat,len);
		res = udp_send(udp_soc,ip,Port,sendbuf,len);
	}
}

U16 udp_callback(U8 socket, U8 *remip, U16 remport, U8 *buf, U16 len)
{
	if(socket != udp_soc) return 0;
	memcpy(Recbuf,buf,len);
	RecLen = len;
	RecLenSum += len;
	Rem_IP[0] = remip[0]; Rem_IP[1] = remip[1]; Rem_IP[2] = remip[2]; Rem_IP[3] = remip[3];	R_Port = remport;
	return 0;
}

int main()
{
	u8 buffer[16];

	SystemInit();
	
	LED_GPIO_Configuration();
	SPI2_Port_Configuration();
	SPI2_Configuration();
	Timer2_Init();
	USART1_Init();
	LCD_Init();
	
	init_TcpNet();
	udp_soc = udp_get_socket (0,UDP_OPT_SEND_CS | UDP_OPT_CHK_CS, udp_callback);
	if(udp_soc) udp_open(udp_soc, LocalPort_NUM);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_8);GPIO_SetBits(GPIOB,GPIO_Pin_9);
	
	while(1)
	{
		if(TimeFlag)
		{
			timer_tick();
			main_TcpNet();
			if(RecLen > 0) //UDP
			{
				GPIO_SetBits(GPIOB,GPIO_Pin_9);
				GPIO_ResetBits(GPIOB,GPIO_Pin_8);
				printf("%s",Recbuf);memset(Recbuf,0,sizeof(u8) * ETH_MTU);
				RecLen = 0;
			}
			else if(USART_RecLen > 0) //USART
			{
				GPIO_SetBits(GPIOB,GPIO_Pin_8);
				GPIO_ResetBits(GPIOB,GPIO_Pin_9);
				UDP_SendData(USART_Recbuf,USART_RecLen,Rem_IP,R_Port);
				USART_RecLen = 0;
			}
			sprintf(buffer, "Net   S:%d R:%d", USART_RecLenSum, RecLenSum);
			LCD_DisplayOneLine(0,buffer);
			sprintf(buffer, "USART S:%d R:%d", RecLenSum, USART_RecLenSum);
			LCD_DisplayOneLine(1,buffer);
			TimeFlag = 0;
		}
	}
}
