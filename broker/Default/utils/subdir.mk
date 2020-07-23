################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/mandar.c \
../utils/memoria.c \
../utils/recibir.c \
../utils/server.c 

OBJS += \
./utils/mandar.o \
./utils/memoria.o \
./utils/recibir.o \
./utils/server.o 

C_DEPS += \
./utils/mandar.d \
./utils/memoria.d \
./utils/recibir.d \
./utils/server.d 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Ipthread -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


