GCC = gcc
FLAGS = -Wall -v
FLAGS_STATIC = -static
DEBUG?=
RM = rm -f
CP = cp -v -f

WIN_GCC = i686-w64-mingw32-gcc
WIN_FLAGS = -Wall -v

OBJECTS=acmedisass.c acmedisass.h

all: acmedisass

acmedisass.o: $(OBJECTS)
	$(GCC) $(FLAGS) $(DEBUG) -c -o $@ $<
	@echo $(OBJECTS)

acmedisass: acmedisass.o
	$(GCC) $(FLAGS) $(DEBUG) -o $@ $<
	$(CP) $@ ../bin/

clean:
	$(RM) acmedisass acmedisass.o
