################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/Camera/camera.c \
../Bsp/Camera/ov2640.c \
../Bsp/Camera/ov2640_regs.c \
../Bsp/Camera/ov5640.c \
../Bsp/Camera/ov5640_regs.c \
../Bsp/Camera/ov7670.c \
../Bsp/Camera/ov7670_regs.c \
../Bsp/Camera/ov7725.c \
../Bsp/Camera/ov7725_regs.c 

OBJS += \
./Bsp/Camera/camera.o \
./Bsp/Camera/ov2640.o \
./Bsp/Camera/ov2640_regs.o \
./Bsp/Camera/ov5640.o \
./Bsp/Camera/ov5640_regs.o \
./Bsp/Camera/ov7670.o \
./Bsp/Camera/ov7670_regs.o \
./Bsp/Camera/ov7725.o \
./Bsp/Camera/ov7725_regs.o 

C_DEPS += \
./Bsp/Camera/camera.d \
./Bsp/Camera/ov2640.d \
./Bsp/Camera/ov2640_regs.d \
./Bsp/Camera/ov5640.d \
./Bsp/Camera/ov5640_regs.d \
./Bsp/Camera/ov7670.d \
./Bsp/Camera/ov7670_regs.d \
./Bsp/Camera/ov7725.d \
./Bsp/Camera/ov7725_regs.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/Camera/%.o Bsp/Camera/%.su Bsp/Camera/%.cyclo: ../Bsp/Camera/%.c Bsp/Camera/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DTFT96 -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp" -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp/Camera" -I"C:/Users/sudha/Documents/STM32/Working project/New project/Bsp/ST7735" -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp-2f-Camera

clean-Bsp-2f-Camera:
	-$(RM) ./Bsp/Camera/camera.cyclo ./Bsp/Camera/camera.d ./Bsp/Camera/camera.o ./Bsp/Camera/camera.su ./Bsp/Camera/ov2640.cyclo ./Bsp/Camera/ov2640.d ./Bsp/Camera/ov2640.o ./Bsp/Camera/ov2640.su ./Bsp/Camera/ov2640_regs.cyclo ./Bsp/Camera/ov2640_regs.d ./Bsp/Camera/ov2640_regs.o ./Bsp/Camera/ov2640_regs.su ./Bsp/Camera/ov5640.cyclo ./Bsp/Camera/ov5640.d ./Bsp/Camera/ov5640.o ./Bsp/Camera/ov5640.su ./Bsp/Camera/ov5640_regs.cyclo ./Bsp/Camera/ov5640_regs.d ./Bsp/Camera/ov5640_regs.o ./Bsp/Camera/ov5640_regs.su ./Bsp/Camera/ov7670.cyclo ./Bsp/Camera/ov7670.d ./Bsp/Camera/ov7670.o ./Bsp/Camera/ov7670.su ./Bsp/Camera/ov7670_regs.cyclo ./Bsp/Camera/ov7670_regs.d ./Bsp/Camera/ov7670_regs.o ./Bsp/Camera/ov7670_regs.su ./Bsp/Camera/ov7725.cyclo ./Bsp/Camera/ov7725.d ./Bsp/Camera/ov7725.o ./Bsp/Camera/ov7725.su ./Bsp/Camera/ov7725_regs.cyclo ./Bsp/Camera/ov7725_regs.d ./Bsp/Camera/ov7725_regs.o ./Bsp/Camera/ov7725_regs.su

.PHONY: clean-Bsp-2f-Camera

