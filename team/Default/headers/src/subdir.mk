################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../headers/src/conexiones_team.c \
../headers/src/entrenadores.c \
../headers/src/pokemon.c 

OBJS += \
./headers/src/conexiones_team.o \
./headers/src/entrenadores.o \
./headers/src/pokemon.o 

C_DEPS += \
./headers/src/conexiones_team.d \
./headers/src/entrenadores.d \
./headers/src/pokemon.d 


# Each subdirectory must supply rules for building sources it contributes
headers/src/%.o: ../headers/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


