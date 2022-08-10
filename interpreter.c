#include <stdio.h>
#include "ks_internal.h"
#include <setjmp.h>

jmp_buf jump_buffer;
volatile double retvalue = 0;

void execute(TreeNode* N);
double executef(TreeNode* N);
long long executei(TreeNode* N);

void execute(TreeNode* N){
	  //int R; //for temporary variable in case (the '=' case)
	  //printf("%d %c %d\n", (int) N->type, N->type, N->leaf_value);
	  //printf("execute\n");
	  switch(N->type){
		  case SETFLOAT:
			symbols[N->args[0]->leaf_value.integer].value.floating = executef(N->args[1]);
			return;
		case SETINT:
			symbols[N->args[0]->leaf_value.integer].value.integer = executei(N->args[1]);
			return;
		case ';':
			execute(N->args[0]);
			execute(N->args[1]);
			return;
		case WHILE:
			while(executef(N->args[0])) execute(N->args[1]);
			execute(N->args[2]);
			return;
		case IF:
			if(executef(N->args[0])) execute(N->args[1]);
			else execute(N->args[2]);
			return;
		case RETURN:
			retvalue = executef(N->args[0]);
			printf("returning %lf\n", retvalue);
			longjmp(jump_buffer, 1);
		}
}

double executef(TreeNode* N){
	  //int R; //for temporary variable in case (the '=' case)
	  //printf("%d %c %d\n", (int) N->type, N->type, N->leaf_value);
	  //printf("executef\n");
	  switch(N->type){
		case 0:
			return 0;
		case IDFLOAT:
			//printf("idfloat %lf\n", symbols[N->leaf_value.integer].value.floating);
			return symbols[N->leaf_value.integer].value.floating;
		case FLOAT:
			return N->leaf_value.floating;
		case IDINT:
			return (double) symbols[N->leaf_value.integer].value.integer;
		case INT:
			return (double) N->leaf_value.integer;
		case '+':
			return executef(N->args[0]) + executef(N->args[1]);
		case '-':
			return executef(N->args[0]) - executef(N->args[1]);
		case '*':
			return executef(N->args[0]) * executef(N->args[1]);
		case '/':
			return executef(N->args[0]) / executef(N->args[1]);
		case SETFLOAT:
			//printf("float %lf\n", 
			symbols[N->args[0]->leaf_value.integer].value.floating = executef(N->args[1]);
			return symbols[N->args[0]->leaf_value.integer].value.floating;
		case SETINT:
			//printf("int (As double)\n");
			return (double) (symbols[N->args[0]->leaf_value.integer].value.integer = executei(N->args[1]));
		case '<':
			return (double) (executef(N->args[0]) < executef(N->args[1]));
		case '>':
			return (double) (executef(N->args[0]) > executef(N->args[1]));

	  }
	  return 0;
}

long long executei(TreeNode* N){
	  //int R; //for temporary variable in case (the '=' case)
	  //printf("%d %c %d\n", (int) N->type, N->type, N->leaf_value);
	  //printf("executei\n");
	  switch(N->type){
		case 0:
			return 0;
		case IDFLOAT:
			return (long long) symbols[N->leaf_value.integer].value.floating;
		case FLOAT:
			return (long long) N->leaf_value.floating;
		case IDINT:
			return symbols[N->leaf_value.integer].value.integer;
		case INT:
			return N->leaf_value.integer;
		case '+':
			return executei(N->args[0]) + executei(N->args[1]);
		case '-':
			return executei(N->args[0]) - executei(N->args[1]);
		case '*':
			return executei(N->args[0]) * executei(N->args[1]);
		case '/':
			return executei(N->args[0]) / executei(N->args[1]);
		case '%':
			return executei(N->args[0]) % executei(N->args[1]);
		case SETFLOAT:
			return (int) (symbols[N->args[0]->leaf_value.integer].value.floating = executef(N->args[1]));
		case SETINT:
			return symbols[N->args[0]->leaf_value.integer].value.integer = executei(N->args[1]);
		case '<':
			return (long long) (executei(N->args[0]) < executei(N->args[1]));
		case '>':
			return (long long) (executei(N->args[0]) > executei(N->args[1]));

	  }
	  return 0;
}

double execute_tree(TreeNode* Head){

	volatile int retstat = 0;
	retvalue = 0;
	printf("setting jump label\n");

	retstat = setjmp(jump_buffer);
	printf("past label\n");
	if(!retstat) {
		execute(Head);
	}
	
	printf("returning retval %lf\n", retvalue);
	return retvalue;
	
}
