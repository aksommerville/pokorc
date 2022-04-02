# build.mk

OPT_AVAILABLE:=$(notdir $(wildcard src/opt/*))
OPT_IGNORE_NATIVE:=$(filter-out $(OPT_ENABLE_NATIVE),$(OPT_AVAILABLE))

SRCFILES:=$(shell find src -type f)

# "embed" data files first run through a binary-binary converter, then get wrapped in C source.
EMBED_SRCFILES:=$(filter src/data/embed/%,$(SRCFILES))
EMBED_CFILES_NATIVE:=$(patsubst src/data/embed/%,mid/native/data/embed/%.c,$(EMBED_SRCFILES))
EMBED_CFILES_TINY:=$(patsubst src/data/embed/%,mid/tiny/data/embed/%.c,$(EMBED_SRCFILES))

# "include" data files get included verbatim.
# We might one day need to convert per-platform; we're ready for it.TODO we do want that; for the tiny menu splash
INCLUDE_SRCFILES:=$(filter src/data/include/%,$(SRCFILES))
INCLUDE_FILES_NATIVE:=$(patsubst src/data/include/%,out/native/data/%,$(INCLUDE_SRCFILES))
INCLUDE_FILES_TINY:=$(patsubst src/data/include/%,out/tiny/data/%,$(INCLUDE_SRCFILES))
all:$(INCLUDE_FILES_NATIVE) $(INCLUDE_FILES_TINY)
out/native/data/%:src/data/include/%;$(PRECMD) cp $< $@
out/tiny/data/%:src/data/include/%;$(PRECMD) cp $< $@

CFILES:=$(filter %.c,$(SRCFILES))
OFILES_NATIVE:=$(filter-out \
  $(addprefix mid/native/opt/,$(addsuffix /%,$(OPT_IGNORE_NATIVE))), \
  $(patsubst src/%.c,mid/native/%.o,$(CFILES)) \
) $(EMBED_CFILES_NATIVE:.c=.o)
OFILES_TOOL_COMMON:=$(filter mid/native/tool/common/%,$(OFILES_NATIVE))
OFILES_GAME:=$(filter mid/native/main/% mid/native/opt/% mid/native/data/embed/%,$(OFILES_NATIVE))
ifneq ($(MAKECMDGOALS),clean)
  -include $(OFILES_NATIVE:.o=d)
endif
mid/native/%.o:src/%.c;$(PRECMD) $(CC_NATIVE) -o $@ $<
mid/native/%.o:mid/native/%.c;$(PRECMD) $(CC_NATIVE) -o $@ $<

define TOOL_RULES
  OFILES_TOOL_$1:=$(filter mid/native/tool/$1/%,$(OFILES_NATIVE)) $(OFILES_TOOL_COMMON)
  TOOL_$1:=out/tool/$1
  all:$$(TOOL_$1)
  $$(TOOL_$1):$$(OFILES_TOOL_$1);$$(PRECMD) $(LD_NATIVE) -o $$@ $$^ $(LDPOST_NATIVE)
endef
TOOLS:=$(filter-out common,$(notdir $(wildcard src/tool/*)))
$(foreach T,$(TOOLS),$(eval $(call TOOL_RULES,$T)))

define EMBED_RULES
  mid/$1/data/embed/%.c:src/data/embed/% $(TOOL_cvtraw);$$(PRECMD) $(TOOL_cvtraw) -o$$@ $$< $2
  mid/$1/data/embed/%.png.c:src/data/embed/%.png $(TOOL_cvtimg);$$(PRECMD) $(TOOL_cvtimg) -o$$@ $$< $2
  mid/$1/data/embed/%.wave.c:src/data/embed/%.wave $(TOOL_mkwave);$$(PRECMD) $(TOOL_mkwave) -o$$@ $$< $2
endef
$(eval $(call EMBED_RULES,native,))
$(eval $(call EMBED_RULES,tiny,--tiny))

all:$(EXE_NATIVE)
$(EXE_NATIVE):$(OFILES_GAME);$(PRECMD) $(LD_NATIVE) -o $@ $(OFILES_GAME) $(LDPOST_NATIVE)

