################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/board.c 

OBJS += \
./Bsp/board.o 

C_DEPS += \
./Bsp/board.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/%.o Bsp/%.su Bsp/%.cyclo: ../Bsp/%.c Bsp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DTFT96 -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp" -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp/Camera" -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp/ST7735" -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp

clean-Bsp:
	-$(RM) ./Bsp/board.cyclo ./Bsp/board.d ./Bsp/board.o ./Bsp/board.su

.PHONY: clean-Bsp

