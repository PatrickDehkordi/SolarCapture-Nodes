#Sample SolarCapture node makefile for reflect
SC_VER_MAJ ?= 1

CFLAGS	:= -Wall -g -O2 -fPIC

SC_LINK := -lsolarcapture${SC_VER_MAJ}


.PHONY: all clean

all: reflect.so reflect_v2.so

clean:
	rm -rf *.o *.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.so: %.o
	$(CC) $(CFLAGS) $(SC_CFLAGS) -shared -Wl,-E \
	  $< $(SC_LINK) -o $@
 
