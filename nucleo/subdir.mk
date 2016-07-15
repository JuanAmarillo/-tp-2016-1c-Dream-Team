################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../archivoLog.c \
../funciones_nucleo.c \
../interfaz.c \
../metadata_program.c \
../nucleo.c \
../parser.c \
../pcb.c \
../planificador.c \
../protocolo_mensaje.c 

OBJS += \
./archivoLog.o \
./funciones_nucleo.o \
./interfaz.o \
./metadata_program.o \
./nucleo.o \
./parser.o \
./pcb.o \
./planificador.o \
./protocolo_mensaje.o 

C_DEPS += \
./archivoLog.d \
./funciones_nucleo.d \
./interfaz.d \
./metadata_program.d \
./nucleo.d \
./parser.d \
./pcb.d \
./planificador.d \
./protocolo_mensaje.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


