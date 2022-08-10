#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ks_exports.h"


int main(int argc, char *argv[])
{
    //puts("Hello, World!");
    
	char* varnames[] = {"w", "h"};
	int vartypes[] = {IDFLOAT, IDINT};
	char* res = compile("area_triangle", 2, varnames, vartypes, "return w*h/2;");
	if(res){printf("Epic compilation fail:\n%s\n", res);return 1;}
	symval varvalues[] = {{.floating = 12.5}, 14};
	res = run_function("area_triangle", varvalues);
	printf("%s\n", res);
	
	/*
	char* varnames2[] = {"l", "s"};
	res = compile("area_rectangle", 2, varnames2, vartypes, "return l*s;");
	if(res){printf("Epic compilation fail:\n%s\n", res);return 1;}
	//int varvalues[] = {12, 14};
	res = run_function("area_rectangle", varvalues);
	printf("%s\n", res);

	int varvalues2[] = {13, 15};
	res = run_function("area_triangle", varvalues2);
	printf("%s\n", res);
	*/
	
	char* rep = eval("int b = 3*2.9; float c = 2.6; float a = b*c; int d = a; return d;");
	printf("result: %s\n", rep);

    return 0;
}
