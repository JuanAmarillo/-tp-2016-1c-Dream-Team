################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../funcionesAuxiliares.c \
../initialize.c \
../protocolo_mensaje.c \
../pruebas.c \
../swap.c 

OBJS += \
./funcionesAuxiliares.o \
./initialize.o \
./protocolo_mensaje.o \
./pruebas.o \
./swap.o 

C_DEPS += \
./funcionesAuxiliares.d \
./initialize.d \
./protocolo_mensaje.d \
./pruebas.d \
./swap.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


