SRC1 = debugger.c
SRC2 = polybugger.c
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)
EXE = $(SRC1:.c=.e)

HFILES = polybugger.h

CFLAGS = -g -Wall --std=c++11

all : $(EXE)

$(EXE) : $(OBJ1) $(OBJ2)
	g++ $(CFLAGS) $(OBJ1) $(OBJ2) -o $(EXE)

$(OBJ1) : $(SRC1) $(HFILES)
	g++ -c $(CFLAGS) $(SRC1) -o $(OBJ1)

$(OBJ2) : $(SRC2) $(HFILES)
	g++ -c $(CFLAGS) $(SRC2) -o $(OBJ2)

clean :
	rm *.o
