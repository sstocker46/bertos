#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
at91sam7x-ek_sd_fat_PROGRAMMER_TYPE = none
at91sam7x-ek_sd_fat_PROGRAMMER_PORT = none

# Files included by the user.
at91sam7x-ek_sd_fat_USER_CSRC = \
	$(at91sam7x-ek_sd_fat_SRC_PATH)/main.c \
	bertos/cpu/arm/drv/spi_dma_at91.c \
	#

# Files included by the user.
at91sam7x-ek_sd_fat_USER_PCSRC = \
	#

# Files included by the user.
at91sam7x-ek_sd_fat_USER_CPPASRC = \
	#

# Files included by the user.
at91sam7x-ek_sd_fat_USER_CXXSRC = \
	#

# Files included by the user.
at91sam7x-ek_sd_fat_USER_ASRC = \
	#

# Flags included by the user.
at91sam7x-ek_sd_fat_USER_LDFLAGS = \
	#

# Flags included by the user.
at91sam7x-ek_sd_fat_USER_CPPAFLAGS = \
	#

# Flags included by the user.
at91sam7x-ek_sd_fat_USER_CPPFLAGS = \
	-fno-strict-aliasing \
	-fwrapv \
	#
