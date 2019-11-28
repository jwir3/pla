PKG_CONFIG = $(shell which pkg-config)
INC = ./include
INC_CAIRO =
LIB_CAIRO =

CFLAGS = -Wall -g -O2 -I$(INC)
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

OBJS = build/render.o build/render_txt.o build/pla.o build/utils.o build/main.o build/load.o build/utf8.o

build/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

pla: $(OBJS)
	$(CC) -o build/pla $(OBJS) $(LDFLAGS)

clean:
	rm -f build/pla $(OBJS)
