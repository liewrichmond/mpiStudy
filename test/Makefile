SHELL = /bin/sh

COLSDIR = ../src/parallel/colSplit
ROWSDIR = ../src/parallel/rowSplit
UTILSDIR = ../src/parallel/utils

all: test_utils test_row_split test_col_split

test_utils: test_utils.c $(UTILSDIR)/utils.c
	gcc test_utils.c $(UTILSDIR)/utils.c -lcheck -o test_utils

test_row_split: test_utils.c $(UTILSDIR)/utils.c $(ROWSDIR)/messages.c
	mpicc test_mpi_rows.c $(ROWSDIR)/messages.c $(UTILSDIR)/utils.c -lcheck -o test_row_split

test_col_split: test_mpi_cols.c $(UTILSDIR)/utils.c $(COLSDIR)/col_utils.c $(COLSDIR)/messages.c
	mpicc test_mpi_cols.c $(UTILSDIR)/utils.c $(COLSDIR)/col_utils.c $(COLSDIR)/messages.c -lcheck -o test_col_split
