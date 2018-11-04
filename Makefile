SRC=main.c
HEADERS=
TARGET=sat_solver
CC=gcc
CC_FLAGS=-Ofast -fopenmp -Wall -Wextra

all: $(SRC) $(HEADERS)
	$(CC) $(CC_FLAGS) $(SRC) -o $(TARGET)
