################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_cache.c \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_common_arm.c \
../drivers/fsl_flexcomm.c \
../drivers/fsl_flexspi.c \
../drivers/fsl_gdma.c \
../drivers/fsl_gpio.c \
../drivers/fsl_i2s_bridge.c \
../drivers/fsl_imu.c \
../drivers/fsl_iped.c \
../drivers/fsl_ocotp.c \
../drivers/fsl_power.c \
../drivers/fsl_reset.c \
../drivers/fsl_usart.c 

C_DEPS += \
./drivers/fsl_cache.d \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_common_arm.d \
./drivers/fsl_flexcomm.d \
./drivers/fsl_flexspi.d \
./drivers/fsl_gdma.d \
./drivers/fsl_gpio.d \
./drivers/fsl_i2s_bridge.d \
./drivers/fsl_imu.d \
./drivers/fsl_iped.d \
./drivers/fsl_ocotp.d \
./drivers/fsl_power.d \
./drivers/fsl_reset.d \
./drivers/fsl_usart.d 

OBJS += \
./drivers/fsl_cache.o \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_common_arm.o \
./drivers/fsl_flexcomm.o \
./drivers/fsl_flexspi.o \
./drivers/fsl_gdma.o \
./drivers/fsl_gpio.o \
./drivers/fsl_i2s_bridge.o \
./drivers/fsl_imu.o \
./drivers/fsl_iped.o \
./drivers/fsl_ocotp.o \
./drivers/fsl_power.o \
./drivers/fsl_reset.o \
./drivers/fsl_usart.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DCPU_RW612ETA2I -DCPU_RW612ETA2I_cm33_nodsp -DMCUXPRESSO_SDK -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1 -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 -DSDK_DEBUGCONSOLE=1 -DMCUX_META_BUILD -DRW612_SERIES -DOSA_USED -DBOOT_HEADER_ENABLE=1 -DSERIAL_PORT_TYPE_UART=1 -DWIFI_BOARD_FRDM_RW61X -DMFLASH_FILE_BASEADDR=7340032 -DCONFIG_NXP_WIFI_SOFTAP_SUPPORT=1 -DSDK_OS_FREE_RTOS -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\flash_config" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\drivers" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\CMSIS" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\CMSIS\m-profile" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\device" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\device\periph" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\drivers\freertos" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\utilities" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\lists" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\utilities\str" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\silicon_id" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\utilities\debug_console" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\serial_manager" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\uart" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\imu_adapter" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\osa\config" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\osa" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\edgefast_wifi\include" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\wifi_bt_module\AzureWave\tx_pwr_limits" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\wifi_bt_module\Murata\tx_pwr_limits" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\wifi_bt_module\u-blox\tx_pwr_limits" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\wifi_bt_module\incl" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClBuffer\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClBuffer\inc\internal" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClCore\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClEls\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClEls\inc\internal" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClMemory\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxClMemory\inc\internal" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslMemory\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslMemory\inc\internal" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslCPreProcessor\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslDataIntegrity\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslFlowProtection\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslParamIntegrity\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\comps\mcuxCsslSecureCounter\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\compiler" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\platforms\rw61x" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\src\platforms\rw61x\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\els_pkc\includes\platform\rw61x" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\flash\mflash" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\flash\mflash\frdmrw612" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\conn_fwloader\include" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\conn_fwloader\fw_bin" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\conn_fwloader\fw_bin\inc" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\conn_fwloader\fw_bin\rw61x" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\component\conn_fwloader\fw_bin\script" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\port\osa" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\port\osa" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\port" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\wifidriver" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\wifi_bt_firmware" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\wifidriver" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\wifidriver\incl" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\wlcmgr" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\wifidriver\wpa_supp_if" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\wifidriver\wpa_supp_if\incl" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\certs" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\firmware_dnld" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\sdio_nxp_abs" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\sdio_nxp_abs\incl" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\port\net" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\port\net" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\wifi\incl\port\net\hooks" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\lwip\port" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\lwip\src\include" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\lwip\port\sys_arch\dynamic" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\freertos\freertos-kernel\include" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\freertos\freertos-kernel\portable\GCC\ARM_CM33_NTZ\non_secure" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\freertos\freertos-kernel\template" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\freertos\freertos-kernel\template\ARM_CM33_3_priority_bits" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\lwip\template" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\source" -I"C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\board" -O0 -fno-common -g3 -gdwarf-4 -c -ffunction-sections -fdata-sections -fno-builtin -mcpu=cortex-m33+nodsp -imacros "C:\Users\fcoda\Documents\MCUXpressoIDE_25.6.136\workspace\frdmrw612_wifi_mqtt\source\mcux_config.h" -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/fsl_cache.d ./drivers/fsl_cache.o ./drivers/fsl_clock.d ./drivers/fsl_clock.o ./drivers/fsl_common.d ./drivers/fsl_common.o ./drivers/fsl_common_arm.d ./drivers/fsl_common_arm.o ./drivers/fsl_flexcomm.d ./drivers/fsl_flexcomm.o ./drivers/fsl_flexspi.d ./drivers/fsl_flexspi.o ./drivers/fsl_gdma.d ./drivers/fsl_gdma.o ./drivers/fsl_gpio.d ./drivers/fsl_gpio.o ./drivers/fsl_i2s_bridge.d ./drivers/fsl_i2s_bridge.o ./drivers/fsl_imu.d ./drivers/fsl_imu.o ./drivers/fsl_iped.d ./drivers/fsl_iped.o ./drivers/fsl_ocotp.d ./drivers/fsl_ocotp.o ./drivers/fsl_power.d ./drivers/fsl_power.o ./drivers/fsl_reset.d ./drivers/fsl_reset.o ./drivers/fsl_usart.d ./drivers/fsl_usart.o

.PHONY: clean-drivers

