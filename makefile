CC = gcc
CFLAGS = -D UNICODE -D _UNICODE -D _WIN32_IE=0x0300 -Wall

LD = gcc
LDLIBS = -lmingw32 -lcomctl32 -lgdi32 -lwinmm -lcomdlg32 -llua
LDFLAGS = -static -mwindows -Wall

SRC=winMain.c winForms.c winMenu.c winDraw.c ogive.c calcSurfaceArea.c calcVolume.c
OBJ=$(SRC:.c=.o)
EXE=fs.exe

all: $(SRC) $(EXE) depend

depend: .depend

.depend: $(SRC)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ >> ./.depend

$(EXE): $(OBJ) RC
	$(LD) $(OBJ) fs.o $(LDFLAGS) $(LDLIBS) -o $@

RC: fs.rc
	windres -i fs.rc -o fs.o

.c.o:   
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY : clean install test

install: all
	mv $(EXE) ../bin

clean:
	rm -f *.o

test: all
	./fs.exe

include .depend
