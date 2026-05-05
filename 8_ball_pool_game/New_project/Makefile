CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Raylib Windows paths (default raylib installer location)
RAYLIB_PATH = C:/raylib/raylib

INCLUDES = -I. -I$(RAYLIB_PATH)/src
LIBS     = -L$(RAYLIB_PATH)/src -lraylib -lopengl32 -lgdi32 -lwinmm

SOURCES = main.c game.c graphics.c physics.c utils.c
OBJECTS = $(SOURCES:.c=.o)
TARGET  = 8ball_pool.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	del /Q *.o $(TARGET) 2>nul || rm -f *.o $(TARGET)
