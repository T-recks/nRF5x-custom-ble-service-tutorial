PROJECT_NAME = $(shell basename "$(realpath ./)")

# Configurations
NRF_IC = nrf52832
SDK_VERSION = 15
SOFTDEVICE_MODEL = s132

# Source and header files
APP_HEADER_PATHS += .
APP_SOURCE_PATHS += .
APP_SOURCES = $(notdir $(wildcard ./*.c))
SRC_FILES += \
	$(NRF_BASE_DIR)/modules/nrfx/drivers/src/nrfx_twi.c \
	$(NRF_BASE_DIR)/modules/nrfx/drivers/src/nrfx_twim.c \
	$(NRF_BASE_DIR)/integration/nrfx/legacy/src/nrf_twi.c \

# Include board Makefile (if any)
include $(NRF_BASE_DIR)/boards/nrf52dk/Board.mk

# Include main Makefile
NRF_BASE_DIR ?= ../../
include $(NRF_BASE_DIR)/make/AppMakefile.mk
