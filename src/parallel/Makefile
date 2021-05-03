SHELL = /bin/bash

COLSDIR = colSplit
ROWSDIR = rowSplit
UTILSDIR = utils

all: gol_rows gol_cols

gol_rows: $(ROWSDIR)/gameOfLife.c $(ROWSDIR)/messages.c $(UTILSDIR)/utils.c
	mpicc $(ROWSDIR)/gameOfLife.c $(ROWSDIR)/messages.c $(UTILSDIR)/utils.c -o gol_rows 

gol_cols: $(COLSDIR)/gameOfLife.c $(COLSDIR)/messages.c $(UTILSDIR)/utils.c $(COLSDIR)/col_utils.c
	mpicc $(COLSDIR)/gameOfLife.c $(COLSDIR)/messages.c $(UTILSDIR)/utils.c $(COLSDIR)/col_utils.c -o gol_cols 
