SRC1 = debugger.c
OBJ1 = $(SRC1:.c=.o)
EXE = $(SRC1:.c=.e)

# HFILES = 

CFLAGS = -g

all : $(EXE)

$(EXE) : $(OBJ1)
	gcc $(CFLAGS) $(OBJ1) -o $(EXE)

$(OBJ1) : $(SRC1)
	gcc -c $(SRC1)
