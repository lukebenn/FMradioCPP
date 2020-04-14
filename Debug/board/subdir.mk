################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/peripherals.c \
../board/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/peripherals.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/peripherals.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_LPC802M001JDH20 -DCPU_LPC802M001JDH20_cm0plus -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/board" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/source" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/drivers" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/device" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/CMSIS" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/component/uart" -I"/Users/luke/Documents/MCUXpresso Workspace/FMradioCPP/utilities" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


