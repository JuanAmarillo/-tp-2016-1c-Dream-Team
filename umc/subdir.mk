################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Interprete.c \
../protocolo_mensaje.c \
../testUMC.c \
../umc.c 

OBJS += \
./Interprete.o \
./protocolo_mensaje.o \
./testUMC.o \
./umc.o 

C_DEPS += \
./Interprete.d \
./protocolo_mensaje.d \
./testUMC.d \
./umc.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


