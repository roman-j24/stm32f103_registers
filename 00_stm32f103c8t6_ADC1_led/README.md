# stm32f103c8t6
# sat 22 mar 2024

pin PA5 - ADC1 button | pin PC13 - LED1 | pin PC14 - LED2

# Compile
arm-none-eabi-as -o crt.o crt.s
# Compile
arm-none-eabi-gcc -mthumb -mcpu=cortex-m3 -O0 -c -o main.o main.c
# Linker
arm-none-eabi-ld -T linker.ld -o main.elf crt.o main.o
# Convert .elf file to .bin
arm-none-eabi-objcopy -O binary main.elf main.bin
# flash STM32F103C8T6
st-flash write main.bin 0x8000000
