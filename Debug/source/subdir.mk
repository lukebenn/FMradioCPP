################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FMradioCPP.cpp \
../source/I2CDisplay.cpp \
../source/arduino.cpp \
../source/cpp_config.cpp \
../source/i2c.cpp \
../source/tuner.cpp 

C_SRCS += \
../source/semihost_hardfault.c 

OBJS += \
./source/FMradioCPP.o \
./source/I2CDisplay.o \
./source/arduino.o \
./source/cpp_config.o \
./source/i2c.o \
./source/semihost_hardfault.o \
./source/tuner.o 

CPP_DEPS += \
./source/FMradioCPP.d \
./source/I2CDisplay.d \
./source/arduino.d \
./source/cpp_config.d \
./source/i2c.d \
./source/tuner.d 

C_DEPS += \
./source/semihost_hardfault.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DCPU_LPC802M001JDH20 -DCPU_LPC802M001JDH20_cm0plus -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/board" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/source" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/drivers" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/device" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/CMSIS" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/component/uart" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/utilities" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fno-rtti -fno-exceptions -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_LPC802M001JDH20 -DCPU_LPC802M001JDH20_cm0plus -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/board" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/source" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/drivers" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/device" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/CMSIS" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/component/uart" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/utilities" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


