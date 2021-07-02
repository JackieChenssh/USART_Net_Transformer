
#include "stm32f10x_map.h"


#define RCC     ((RCC_TypeDef *) RCC_BASE)
#define FLASH   ((FLASH_TypeDef *) FLASH_BASE)

void SystemInit(void)  //Config CLK SYSTEM 
{
   u32 tmp=0;
	/* Reset HSEON bit */
	 RCC->CR &= 0xFFFEFFFF;
	/* Reset HSEBYP bit */
	 RCC->CR &= 0xFFFBFFFF;
	/* Set HSEON bit */
	 RCC->CR |= 0x00010000;
	
  while((RCC->CR & 0x00010000)==0); //wait HSE Set
	
	RCC->CFGR &= 0xFFC0C00F;  // HCLKConfig PCLK2Config PCLK1Config
	RCC->CFGR |= 0x00DD0400;
	
	FLASH->ACR &= 0x00000028; // FLASH_SetLatency  PrefetchBuffer
	FLASH->ACR |= 0x00000012;
	
	RCC->CR |= 0x01000000; //PLL ON
	
	while((RCC->CR & 0x02000000)==0); //wait PLL Set
	
	tmp=RCC->CFGR;  //SYSCLK Switch to PLL
	tmp &= 0xFFFFFFFC;
	tmp |=0x00000002;
	RCC->CFGR = tmp;
	
	while((RCC->CFGR & 0x0000000C)!= 0x08); //wait SYSCLK Set

}


