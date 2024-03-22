// Created wed 20 mar 2024
// arm-none-eabi-as -o crt.o crt.s && arm-none-eabi-gcc -mthumb -mcpu=cortex-m3 -O0 -c -o main.o main.c && arm-none-eabi-ld -T linker.ld -o main.elf crt.o main.o && arm-none-eabi-objcopy -O binary main.elf main.bin && st-flash write main.bin 0x8000000

#include <stdint.h>

#define NVIC_BASE   (*((volatile uint32_t *)0xE000E100))

#define RCC_CFGR    (*((volatile uint32_t *)0x40021004))
#define RCC_APB2ENR (*((volatile uint32_t *)0x40021018))

#define GPIOA_CRL   (*((volatile uint32_t *)0x40010800))
#define GPIOC_CRH   (*((volatile uint32_t *)0x40011004))
#define GPIOx_BRR   (*((volatile uint32_t *)0x40011014))
#define GPIOC_ODR   (*((volatile uint32_t *)0x4001100C))

#define ADC1_SR    (*((volatile uint32_t *)0x40012400))
#define ADC1_CR1   (*((volatile uint32_t *)0x40012404))
#define ADC1_CR2   (*((volatile uint32_t *)0x40012408))
#define ADC1_SMPR2 (*((volatile uint32_t *)0x4001240C))
#define ADC_SQR3   (*((volatile uint32_t *)0x40012434))
#define ADC1_DR    (*((volatile uint32_t *)0x4001244C))

uint32_t val_adc = 0;

void ADC1_2_IRQHandler(void)
{
	if (ADC1_SR & (1 << 1))
		val_adc = ADC1_DR;
}

int main()
{
	// Configuration PA5 as analog input ADC1
	RCC_APB2ENR |= (1 << 9) | (1 << 2) | (1 << 0);	// ADC1EN, AFIOEN, IOPAEN
	RCC_CFGR    |= (0b10 << 14);			// ADCPRE: ADC prescaler, 0b10: PCLK2 divided by 6
	GPIOA_CRL   |= (0b00 << 22);			// CNF5[1:0], 0b00: Analog mode

	ADC1_CR1    |= (1 << 5);                        // EOCIE: Interrupt enable for EOC
	NVIC_BASE   |= (1 << 18);                       // Enable ADC interrupt in NVIC

	ADC1_SMPR2  |= (0b111 << 15);                   // SMP5[2:1], 0b111: 239.5 cycles
	ADC_SQR3    |= (0b0101 << 0);                   // SQ1[4:0], first conversion in regular sequence

	ADC1_CR2    |= (1 << 0) | (1 << 1);             // ADON, CONT: Continuous conversion
	ADC1_CR2    |= (1 << 0);                        // ADON: A/D converter ON / OFF, second time

	ADC1_CR2    |= (1 << 2);                        // CAL: A/D Calibration
	while ((ADC1_CR2 & (1 << 2)) == 0);             // Waiting for the end of calibration

	// Configuration PC13, PC14 as output
	RCC_APB2ENR |= (1 << 4);			// IOPCEN: I/O port C clock enable
	GPIOC_CRH   &= (~(0xF << 20)) & (~(0xF << 24)); // CNF13[1:0], CNF14[1:0],
							// 0b00: General purpose output push-pull

	GPIOC_CRH   |= (2 << 20) | (2 << 24);		// MODE13[1:0], MODE14[1:0],
							// 10: Output mode, max speed 2 MHz.

	while (1)
	{
		if (val_adc > 1000)
		{
			GPIOx_BRR |= (1 << 14); // reset PC14
			GPIOC_ODR ^= (1 << 13); // toggle PC13
			for (uint32_t i=0; i<32000*1; i++);
		}
		else
		{
			GPIOx_BRR |= (1 << 13); // reset PC13
			GPIOC_ODR ^= (1 << 14); // toggle PC14
			for (uint32_t i=0; i<32000*2; i++);
		}
	}

	return 0;
}
