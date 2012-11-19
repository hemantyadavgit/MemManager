
CC=gcc
CFLAGS=
LDFLAGS+=
LIB32=/usr/lib
LIB64=/usr/lib
SOURCES=
OBJ=mm_public.o main_mm packet_public.o go_public

all:    mm packet


run:    mm
	./main_mm
   
   
   
mm: mm_public.o
	gcc -o main_mm main_mm.c mm_public.o

mm_public.o:
	gcc -c mm_public.c
   
packet: packet_public.o
	gcc -o pack_pub packet_public.o mm_public.o

packet_public.o:
	gcc -c packet_public.c mm_public.c

clean:
	rm -rf $(OBJ)
