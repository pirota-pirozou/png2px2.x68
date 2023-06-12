CC = gcc
CFLAGS = -O -DALLMEM -DBIG_ENDIAN -cpp-stack=409600 -IB:/INCLUDE
LIBDIR = B:/LIB
LIBS = $(addprefix $(LIBDIR)/, libz.a libpng.a FLOATFNC.L)

LDFLAGS = -cpp-stack=409600
INCLUDE = -I./

.PHONY: clean

EXECUTABLE = PNG2PX2.x

all: $(EXECUTABLE)

OBJS = PNG2PX2.o pngctrl.o

$(EXECUTABLE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c pngctrl.h
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -f $(EXECUTABLE) $(OBJS)
