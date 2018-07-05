################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../controllerHandler/CController.cpp 

OBJS += \
./controllerHandler/CController.o 

CPP_DEPS += \
./controllerHandler/CController.d 


# Each subdirectory must supply rules for building sources it contributes
controllerHandler/%.o: ../controllerHandler/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


