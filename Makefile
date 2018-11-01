SRC=main.c
HEADERS=
TARGET=sat_solver
CC=gcc
CC_FLAGS=-pthread

all: $(SRC) $(HEADERS)
	$(CC) $(CC_FLAGS) $(SRC) -o $(TARGET)
