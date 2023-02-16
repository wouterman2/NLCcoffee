################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/airpump.c \
../Core/Src/am2302.c \
../Core/Src/analog.c \
../Core/Src/dma.c \
../Core/Src/driptray.c \
../Core/Src/gpio.c \
../Core/Src/heaters.c \
../Core/Src/interrupt.c \
../Core/Src/iwdg.c \
../Core/Src/main.c \
../Core/Src/pumps.c \
../Core/Src/recipe.c \
../Core/Src/ritualbutton.c \
../Core/Src/spi.c \
../Core/Src/steamwand.c \
../Core/Src/stm32l1xx_hal_msp.c \
../Core/Src/stm32l1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l1xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c \
../Core/Src/valves.c \
../Core/Src/watertank.c \
../Core/Src/work.c \
../Core/Src/zerocross.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/airpump.o \
./Core/Src/am2302.o \
./Core/Src/analog.o \
./Core/Src/dma.o \
./Core/Src/driptray.o \
./Core/Src/gpio.o \
./Core/Src/heaters.o \
./Core/Src/interrupt.o \
./Core/Src/iwdg.o \
./Core/Src/main.o \
./Core/Src/pumps.o \
./Core/Src/recipe.o \
./Core/Src/ritualbutton.o \
./Core/Src/spi.o \
./Core/Src/steamwand.o \
./Core/Src/stm32l1xx_hal_msp.o \
./Core/Src/stm32l1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l1xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o \
./Core/Src/valves.o \
./Core/Src/watertank.o \
./Core/Src/work.o \
./Core/Src/zerocross.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/airpump.d \
./Core/Src/am2302.d \
./Core/Src/analog.d \
./Core/Src/dma.d \
./Core/Src/driptray.d \
./Core/Src/gpio.d \
./Core/Src/heaters.d \
./Core/Src/interrupt.d \
./Core/Src/iwdg.d \
./Core/Src/main.d \
./Core/Src/pumps.d \
./Core/Src/recipe.d \
./Core/Src/ritualbutton.d \
./Core/Src/spi.d \
./Core/Src/steamwand.d \
./Core/Src/stm32l1xx_hal_msp.d \
./Core/Src/stm32l1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l1xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d \
./Core/Src/valves.d \
./Core/Src/watertank.d \
./Core/Src/work.d \
./Core/Src/zerocross.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L151xC -c -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I"../Core shared/inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/airpump.d ./Core/Src/airpump.o ./Core/Src/airpump.su ./Core/Src/am2302.d ./Core/Src/am2302.o ./Core/Src/am2302.su ./Core/Src/analog.d ./Core/Src/analog.o ./Core/Src/analog.su ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/driptray.d ./Core/Src/driptray.o ./Core/Src/driptray.su ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/heaters.d ./Core/Src/heaters.o ./Core/Src/heaters.su ./Core/Src/interrupt.d ./Core/Src/interrupt.o ./Core/Src/interrupt.su ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pumps.d ./Core/Src/pumps.o ./Core/Src/pumps.su ./Core/Src/recipe.d ./Core/Src/recipe.o ./Core/Src/recipe.su ./Core/Src/ritualbutton.d ./Core/Src/ritualbutton.o ./Core/Src/ritualbutton.su ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/steamwand.d ./Core/Src/steamwand.o ./Core/Src/steamwand.su ./Core/Src/stm32l1xx_hal_msp.d ./Core/Src/stm32l1xx_hal_msp.o ./Core/Src/stm32l1xx_hal_msp.su ./Core/Src/stm32l1xx_it.d ./Core/Src/stm32l1xx_it.o ./Core/Src/stm32l1xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l1xx.d ./Core/Src/system_stm32l1xx.o ./Core/Src/system_stm32l1xx.su ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/valves.d ./Core/Src/valves.o ./Core/Src/valves.su ./Core/Src/watertank.d ./Core/Src/watertank.o ./Core/Src/watertank.su ./Core/Src/work.d ./Core/Src/work.o ./Core/Src/work.su ./Core/Src/zerocross.d ./Core/Src/zerocross.o ./Core/Src/zerocross.su

.PHONY: clean-Core-2f-Src

