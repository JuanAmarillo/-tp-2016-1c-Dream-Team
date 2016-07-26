################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../E-S.c \
../archivoLog.c \
../funciones_nucleo.c \
../interfaz.c \
../metadata_program.c \
../nucleo.c \
../parser.c \
../pcb.c \
../planificador.c \
../protocolo_mensaje.c \
../semaforos.c \
../variables_compartidas.c 

OBJS += \
./E-S.o \
./archivoLog.o \
./funciones_nucleo.o \
./interfaz.o \
./metadata_program.o \
./nucleo.o \
./parser.o \
./pcb.o \
./planificador.o \
./protocolo_mensaje.o \
./semaforos.o \
./variables_compartidas.o 

C_DEPS += \
./E-S.d \
./archivoLog.d \
./funciones_nucleo.d \
./interfaz.d \
./metadata_program.d \
./nucleo.d \
./parser.d \
./pcb.d \
./planificador.d \
./protocolo_mensaje.d \
./semaforos.d \
./variables_compartidas.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


