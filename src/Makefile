CC=gcc
CFLAGS=-g -Wall `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0`
LDFLAGS=-g -Wall `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -lm

OBJ=tcg.o ps.o tcg_path.o tcg_connect.o tcg_util.o tcg_tig.o tcg_aig.o 
OBJ+=tce.o tce_draw.o

tce: $(OBJ)
	echo $(OBJ)
	${CC} -o $@ $(OBJ) ${LDFLAGS}


clean::
	-rm $(OBJ) tce
	
zip::
	zip tooleval.zip Makefile *.[hc] *.glade
