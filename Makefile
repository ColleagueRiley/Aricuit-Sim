CC = gcc
NAME = Aircuit


LIBS := -lopengl32 -lshell32 -lgdi32 -lm --static
OUTPUT = $(NAME).exe

ifneq (,$(filter $(CC),winegcc x86_64-w64-mingw32-gcc))
    detected_OS := Windows
else
ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif
endif

OUTPUT = $(NAME).exe

ifeq ($(detected_OS),Windows)
	LIBS := -lopengl32 -lshell32 -lgdi32 -lm --static
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVide
	OUTPUT = $(NAME)
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lX11 -lGL -lm 
	OUTPUT = $(NAME)
endif

all:
	make RSGL-$(detected_OS).o
	$(CC) source/main.c -O3 -Wall -I./include $(LIBS) RSGL-$(detected_OS).o -o $(OUTPUT) 

debug:
	make RSGL-$(detected_OS).o
	$(CC) source/main.c RSGL-$(detected_OS).o -Wall -g -I./include $(LIBS) -o $(OUTPUT)
	./$(OUTPUT)

RSGL-$(detected_OS).o:
	cp include/RSGL.h RSGL.c
	$(CC) -c -fPIC RSGL.c -Wall -D RSGL_IMPLEMENTATION -D RGFW_NO_JOYSTICK_CODES -I./include/ -o RSGL-$(detected_OS).o
	rm RSGL.c

clean:
	rm $(OUTPUT)