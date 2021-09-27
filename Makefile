TARGET=raytracer
CC=gcc

SRC = raytracer.c geometry.c scene.c
OBJ = raytracer.o geometry.o scene.o
HDR = geometry.h scene.h
CFLAGS = -std=c11
CFLAGS += -Wall -Wextra
#CFLAGS += -g # debug
CFLAGS += -O3 # Release

# no linker flags
LFLAGS =

all: $(TARGET)


$(TARGET): Makefile $(HDR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LFLAGS)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)


clean:
	rm -f $(TARGET) $(OBJ)
