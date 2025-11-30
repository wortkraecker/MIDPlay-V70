TARGET = SpotifyConnectPSP
OBJS = src/main.o src/wifi.o src/utils.o src/http_client.o src/mp3_decoder.o src/psp_audio.o src/audio_stream.o src/metadata.o src/ui.o
INCDIR = src minimp3
CFLAGS = -G0 -O2 -Wall -fno-exceptions -fno-rtti
CXXFLAGS = $(CFLAGS)
LIBS = -lpspnet -lpsphttp -lpspssl -lpspnet_apctl -lpspnet_inet -lpspgu -lpspgum -lpspaudio -lpspaudiolib -lpspdebug -lstdc++

BUILD_PRX = 1
PSP_FW_VERSION = 371

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Spotify Connect PSP
PSP_EBOOT_ICON = res/icon0.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
