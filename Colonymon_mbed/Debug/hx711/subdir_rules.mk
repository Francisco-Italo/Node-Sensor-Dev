################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
hx711/%.obj: ../hx711/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/home/italo/ti/ccs1270/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --use_hw_mpy=F5 --include_path="/home/italo/ti/ccs1270/ccs/ccs_base/msp430/include" --include_path="/home/italo/workspace_v12/Colonymon_mbed" --include_path="/home/italo/ti/ccs1270/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR2433__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="hx711/$(basename $(<F)).d_raw" --obj_directory="hx711" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


