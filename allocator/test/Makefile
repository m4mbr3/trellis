all:
	gcc -g -I../include -L../lib -o test_lib_shared test_lib.c -lprivsep_malloc
	gcc -g --static -I../include -L../lib -o test_lib_static test_lib.c -lprivsep_malloc

