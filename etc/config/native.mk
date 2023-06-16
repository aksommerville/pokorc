# native.mk
# Rules for building the game for your PC.

PO_NATIVE_PLATFORM:=raspi

ifeq ($(PO_NATIVE_PLATFORM),linux) #-----------------------------------------------

  CC_NATIVE:=gcc -c -MMD -O2 -Isrc -Isrc/main -Werror -Wimplicit -DPO_NATIVE=1 -I/usr/include/libdrm
  LD_NATIVE:=gcc
  LDPOST_NATIVE:=-lm -lz -lasound -lX11 -lpthread -ldrm
  OPT_ENABLE_NATIVE:=genioc alsa x11 drmfb evdev
  OPT_ENABLE_TOOL:=alsa ossmidi inotify
  EXE_NATIVE:=out/native/pokorc

else ifeq ($(PO_NATIVE_PLATFORM),raspi) #-----------------------------------------
# Newer Raspberry Pi should use the 'linux' platform, they should have a usable DRM implementation.
# Our 'raspi' platform is for older models that need to use Broadcom's video.
# Beyond that, raspi and linux are the same thing.

  CC_NATIVE:=gcc -c -MMD -O2 -Isrc -Isrc/main -Werror -Wimplicit -DPO_NATIVE=1 -I/opt/vc/include
  LD_NATIVE:=gcc -L/opt/vc/lib
  LDPOST_NATIVE:=-lm -lz -lasound -lpthread -lbcm_host -lEGL -lGLESv2 -lGL
  OPT_ENABLE_NATIVE:=genioc alsa evdev bcm
  OPT_ENABLE_TOOL:=alsa ossmidi inotify
  EXE_NATIVE:=out/native/pokorc

else ifeq ($(PO_NATIVE_PLATFORM),macos) #------------------------------------------

  $(error TODO Build for MacOS)
  
else ifneq ($(PO_NATIVE_PLATFORM),mswin) #-----------------------------------------

  $(error TODO Build for Windows)

else ifeq ($(PO_NATIVE_PLATFORM),) #-----------------------------------------------
else
  $(error Undefined native platform '$(PO_NATIVE_PLATFORM)')
endif
