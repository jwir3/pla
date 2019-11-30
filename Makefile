VERSION_MAJOR=1
VERSION_MINOR=0
VERSION_PATCH=0
PKG_CONFIG = $(shell which pkg-config)
INC = ./include
INC_CAIRO =
LIB_CAIRO =
LN = `which ln`
CP = `which cp`
RM = `which rm`

CFLAGS = -Wall -g -O2 -fpic -I$(INC)
LDFLAGS = -lm

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
  LDFLAGS += -l$(LIB_CAIRO)
else
  ifneq ($(PKG_CONFIG),)
    LDFLAGS += $(shell $(PKG_CONFIG) --libs cairo)
  else
    LDFLAGS += -lcairo
  endif
endif

OBJS = build/render.o build/render_txt.o build/pla.o build/utils.o build/load.o build/utf8.o
MAIN = build/main.o

pla: build $(MAIN) lib
	$(CC) -o build/pla build/main.o -L./build -lpla

build:
	mkdir -p build

build/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

lib: $(OBJS)
	$(CC) -shared -o build/libpla.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OBJS) $(LDFLAGS)

main.o:
	$(CC) -c -o build/pla src/main.c -lpla -Wall -g -O2 -I$(INC)

install: lib
	$(CP) -f build/libpla.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) /usr/local/lib
	$(RM) -f /usr/local/lib/libpla.so.$(VERSION_MAJOR) /usr/local/lib/libpla.so
	$(LN) -s /usr/local/lib/libpla.so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) /usr/local/lib/libpla.so.$(VERSION_MAJOR)
	$(LN) -s /usr/local/lib/libpla.so.$(VERSION_MAJOR) /usr/local/lib/libpla.so

clean:
	rm -f build/pla build/libpla.* build/*.o
