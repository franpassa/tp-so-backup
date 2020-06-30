################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../servidor/servidor.c \
../servidor/utils.c 

OBJS += \
./servidor/servidor.o \
./servidor/utils.o 

C_DEPS += \
./servidor/servidor.d \
./servidor/utils.d 


# Each subdirectory must supply rules for building sources it contributes
servidor/%.o: ../servidor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


