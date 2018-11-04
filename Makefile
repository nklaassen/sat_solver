SRC=main.c
HEADERS=
TARGET=sat_solver
CC=gcc
CC_FLAGS=-pthread -std=c99

all: $(SRC) $(HEADERS)
	$(CC) $(CC_FLAGS) $(SRC) -o $(TARGET)
