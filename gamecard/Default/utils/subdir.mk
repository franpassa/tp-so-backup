################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/comunicacion.c \
../utils/coordenadas.c \
../utils/filesystem.c \
../utils/general.c \
../utils/pokemon.c 

OBJS += \
./utils/comunicacion.o \
./utils/coordenadas.o \
./utils/filesystem.o \
./utils/general.o \
./utils/pokemon.o 

C_DEPS += \
./utils/comunicacion.d \
./utils/coordenadas.d \
./utils/filesystem.d \
./utils/general.d \
./utils/pokemon.d 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O1 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


