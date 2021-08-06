SRC1 = debugger.c
SRC2 = polybugger.c
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)
EXE = $(SRC1:.c=.e)

HFILES = polybugger.h

CFLAGS = -g -Wall --std=c11

all : $(EXE)

$(EXE) : $(OBJ1) $(OBJ2)
	gcc $(CFLAGS) $(OBJ1) $(OBJ2) -o $(EXE)

$(OBJ1) : $(SRC1) $(HFILES)
	gcc -c $(CFLAGS) $(SRC1) -o $(OBJ1)

$(OBJ2) : $(SRC2) $(HFILES)
	gcc -c $(CFLAGS) $(SRC2) -o $(OBJ2)

