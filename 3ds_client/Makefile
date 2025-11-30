# 3DS Spotify Connect client Makefile (2025 devkitPro layout)
# Requires: devkitPro (dkp-pacman), devkitARM, libctru, citro2d, citro3d, ndsp, httpc, soc.

TARGET        := 3ds_spotify
BUILD         := build
SOURCES       := src
INCLUDES      := src minimp3
ROMFS         := romfs
APP_TITLE     := Spotify Connect 3DS
APP_DESCRIPTION := Stream and control Spotify Connect via remote server
APP_AUTHOR    := MIDPlay
APP_VERSION   := 0.2.0

# pull in common rules
include $(DEVKITPRO)/libctru/3ds_rules

# libraries (order matters)
LIBS          := -lcitro2d -lcitro3d -lctru -lhttpc -lsoc -lm -lz
# enable if you swap sockets for mbedtls/curl
# LIBS        += -lmbedtls -lmbedx509 -lmbedcrypto -lcurl

CFLAGS        := -Wall -Wextra -Wno-psabi -O2 -std=c11 -ffunction-sections -fdata-sections
CXXFLAGS      := -Wall -Wextra -Wno-psabi -O2 -std=c++17 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions
CFLAGS        += $(INCLUDE)
CXXFLAGS      += $(INCLUDE)

LDFLAGS       := -specs=3dsx.specs -g -Wl,--gc-sections

.PHONY: all cia clean
all: $(TARGET).3dsx
cia: $(TARGET).cia

# Optional CIA packaging (bannertool + makerom must be in PATH)
icon.icn: romfs/app.icon.png
bannertool makesmdh -s "$(APP_TITLE)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i $< -o $@

banner.bnr: romfs/banner.png
bannertool makebanner -i $< -a romfs/banner_audio.wav -o $@

$(TARGET).cia: $(TARGET).3dsx icon.icn banner.bnr
makerom -f cia -o $@ -DAPP_ENCRYPTED=false -rsf cia.rsf -target t -exefslogo -banner banner.bnr -icon icon.icn -elf $(TARGET).elf -romfs $(ROMFS)

clean:
@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).cia icon.icn banner.bnr
