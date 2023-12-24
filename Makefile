CC = gcc
LIBS := -lopengl32 -lshell32 -lgdi32 -lm

ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(detected_OS),Windows)
	LIBS := -lopengl32 -lshell32 -lgdi32 -lm
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lX11 -lGL -lm 
endif

OUTPUT = aircuit

all:
	make RSGL.o
	$(CC) source/main.c -O3 -Wall -I./include $(LIBS) RSGL.o -o $(OUTPUT) 

debug:
	make RSGL.o
	$(CC) source/main.c -Wall -lGL -g -I./include $(LIBS) RSGL.o -o $(OUTPUT)
	./$(OUTPUT)

RSGL.o:
	cp include/RSGL.h RSGL.c
	$(CC) -c RSGL.c -Wall -D RSGL_IMPLEMENTATION -D RGFW_NO_JOYSTICK_CODES -I./include/
	rm RSGL.c


clean:
	rm $(OUTPUT)