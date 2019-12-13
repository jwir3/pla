VERSION_MAJOR=1
VERSION_MINOR=0
VERSION_PATCH=0

PKG_CONFIG = $(shell which pkg-config)
LN = `which ln`
CP = `which cp`
RM = `which rm`
MKDIR = `which mkdir`

BIN_LIB_DIR = $(shell pwd)/build/lib

INC = ./include
INC_CAIRO =
LIB_CAIRO =
LIB_CFLAGS = -fpic
CFLAGS = -Wall -g -O2 -I$(INC)
LIB_LDFLAGS = -lm
BIN_LDFLAGS = -L$(BIN_LIB_DIR) -lpla

ifneq ($(INC_CAIRO),)
  CFLAGS += -I$(INC_CAIRO)
else
  ifneq ($(PKG_CONFIG),)
    CFLAGS += $(shell $(PKG_CONFIG) --cflags cairo)
  else
    CFLAGS += -I/usr/include/cairo
  endif
endif

ifneq ($(LIB_CAIRO),)
  LIB_LDFLAGS += -l$(LIB_CAIRO)
else
  ifneq ($(PKG_CONFIG),)
    LIB_LDFLAGS += $(shell $(PKG_CONFIG) --libs cairo)
  else
    LIB_LDFLAGS += -lcairo
  endif
endif

OBJS = build/lib/render.o build/lib/render_txt.o build/lib/pla.o build/lib/utils.o build/lib/load.o build/lib/utf8.o
MAIN = build/main.o

all: lib bin

setup:
	$(MKDIR) -p build/lib

bin: $(MAIN)
	$(CC) -o build/pla build/main.o $(BIN_LDFLAGS)

build/main.o:
	$(CC) -c -o build/main.o src/main.c $(CFLAGS)

build/lib/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_CFLAGS)

lib: setup build-lib create-lib-links

build-lib: setup $(OBJS)
	$(CC) -shared -o build/lib/libpla.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OBJS) $(LIB_LDFLAGS)

remove-lib-links:
	$(RM) -f build/lib/libpla.so build/lib/libpla.so.$(VERSION_MAJOR)

create-lib-links: remove-lib-links
	$(LN) -s $(BIN_LIB_DIR)/libpla.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(BIN_LIB_DIR)/libpla.so.$(VERSION_MAJOR)
	$(LN) -s $(BIN_LIB_DIR)/libpla.so.$(VERSION_MAJOR) $(BIN_LIB_DIR)/libpla.so

clean:
	$(RM) -rf build
