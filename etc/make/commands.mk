# commands.mk

run:$(EXE_NATIVE) $(INCLUDE_FILES_NATIVE);$(EXE_NATIVE)

launch:$(TINY_BIN_SOLO); \
  stty -F /dev/$(TINY_PORT) 1200 ; \
  sleep 2 ; \
  $(TINY_PKGROOT)/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(TINY_PORT) -U true -i -e -w $(TINY_BIN_SOLO) -R

sdcard:$(TINY_PACKAGE);etc/tool/sdcard.sh $(TINY_PACKAGE)

clean:;rm -rf mid out

test:;echo "TODO: make $@" ; exit 1
