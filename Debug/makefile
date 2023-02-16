################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32L1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Core\ shared/src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := 80482201 - CoffeeMachine
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
80482201\ -\ CoffeeMachine.elf \

MAP_FILES += \
80482201\ -\ CoffeeMachine.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
80482201\ -\ CoffeeMachine.list \

OBJCOPY_HEX += \
80482201\ -\ CoffeeMachine.hex \

OBJCOPY_BIN += \
80482201\ -\ CoffeeMachine.bin \


# All Target
all: main-build

# Main-build Target
main-build: 80482201\ -\ CoffeeMachine.elf secondary-outputs

# Tool invocations
80482201\ -\ CoffeeMachine.elf 80482201\ -\ CoffeeMachine.map: $(OBJS) $(USER_OBJS) D:\Github\nunc-coffee-embedded\STM32L151RCTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "80482201 - CoffeeMachine.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\Github\nunc-coffee-embedded\STM32L151RCTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="80482201 - CoffeeMachine.map" -Wl,--gc-sections -static -L"D:\Trial\nunc-coffee-embedded\Libraries\Nunc" --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

80482201\ -\ CoffeeMachine.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "80482201 - CoffeeMachine.list"
	@echo 'Finished building: $@'
	@echo ' '

80482201\ -\ CoffeeMachine.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "80482201 - CoffeeMachine.hex"
	@echo 'Finished building: $@'
	@echo ' '

80482201\ -\ CoffeeMachine.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "80482201 - CoffeeMachine.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) 80482201\ -\ CoffeeMachine.bin 80482201\ -\ CoffeeMachine.elf 80482201\ -\ CoffeeMachine.hex 80482201\ -\ CoffeeMachine.list 80482201\ -\ CoffeeMachine.map 80482201\\ -\\ CoffeeMachine.elf default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
