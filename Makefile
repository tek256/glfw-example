CC_FLAGS = -w -std=c99 -O2
EXEC_NAME = example

ifeq ($(OS),Windows_NT)
	LD_FLAGS := -lopengl32 -lglfw3 -lgdi32 -lm -lopenal32
	EXEC_NAME := $(EXEC_NAME).exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
			LD_FLAGS := -lglfw -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -lXcursor -ldl -lm -g 
		endif
    ifeq ($(UNAME_S),Darwin)
			LD_FLAGS := -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -lm 
    endif
endif

all : $(OBJS)
	cc src/main.c $(CC_FLAGS) $(LD_FLAGS) -Idep/ -o $(EXEC_NAME)
