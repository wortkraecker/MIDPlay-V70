# 3DS Spotify Connect client Makefile
# Requires devkitPro with devkitARM, libctru, citro2d/citro3d, and zlib.

include $(DEVKITPRO)/libctru/3ds_rules

TARGET:= 3ds_spotify
BUILD:= build
SOURCES:= src
INCLUDES:= src minimp3
ROMFS:= romfs

LIBS:= -lcitro2d -lcitro3d -lctru -lm -lz
# mbedTLS may be used for HTTPS metadata if needed
# LIBS      += -lmbedtls -lmbedx509 -lmbedcrypto

CFLAGS:= -Wall -Wextra -O2 -std=c++17 -ffunction-sections -fdata-sections -D__3DS__
CFLAGS+= $(INCLUDE) -DMINIMP3_IMPLEMENTATION
CXXFLAGS:= $(CFLAGS)

LDFLAGS:= -specs=3dsx.specs -g -Wl,--gc-sections

# Assets copied into romfs (e.g., default cover placeholder)
# Create romfs directory if you want bundled assets

# Additional libraries
LIBDIRS:=

.PHONY: all cia clean

all: $(TARGET).3dsx

cia: $(TARGET).cia

$(BUILD):
@mkdir -p $@

# Use built-in 3ds_rules logic

# CIA packaging using bannertool + makerom (must be installed)
icon.icn: romfs/app.icon.png
bannertool makesmdh -s "Spotify Connect" -l "3DS Client" -p "MIDPlay" -i $< -o $@

banner.bnr: romfs/banner.png
bannertool makebanner -i $< -a romfs/banner_audio.wav -o $@

$(TARGET).cia: $(TARGET).3dsx icon.icn banner.bnr
makerom -f cia -o $@ -DAPP_ENCRYPTED=false -rsf cia.rsf -target t -exefslogo -banner banner.bnr -icon icon.icn -elf $(TARGET).elf -romfs $(ROMFS)

clean:
@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).cia icon.icn banner.bnr
