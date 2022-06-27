#include <stdio.h>
#include "ks_internal.h"
#include <setjmp.h>

jmp_buf jump_buffer;
volatile char returned = (char) 0;

int execute1(TreeNode* N){
	  int R; //for temporary variable in case (the '=' case)
	  printf("%d %c %d\n", (int) N->type, N->type, N->leaf_value);
	  switch(N->type){
		case 0:
			return 0;
		case ID:
			return symbols[N->leaf_value].value;
		case INT:
			return N->leaf_value;
		case '+':
			return execute1(N->args[0]) + execute1(N->args[1]);
		case '-':
			return execute1(N->args[0]) - execute1(N->args[1]);
		case '*':
			return execute1(N->args[0]) * execute1(N->args[1]);
		case '/':
			return execute1(N->args[0]) / execute1(N->args[1]);
		case '%':
			return execute1(N->args[0]) % execute1(N->args[1]);
		case '=':
			R = execute1(N->args[1]);
			symbols[N->args[0]->leaf_value].value = R;
			return R;
		case '<':
			return (int) (execute1(N->args[0]) < execute1(N->args[1]));
		case '>':
			return (int) (execute1(N->args[0]) > execute1(N->args[1]));
		case ';':
			execute1(N->args[0]);
			return execute1(N->args[1]);
		case WHILE:
			while(execute1(N->args[0])) execute1(N->args[1]);
			return execute1(N->args[2]);
		case IF:
			if(execute1(N->args[0])) execute1(N->args[1]);
			return execute1(N->args[2]);
		case RETURN:
			returned = 1;
			longjmp(jump_buffer, execute1(N->args[0]));

	  }
	  return 0;
}

int execute(TreeNode* Head){

	volatile int retval = 0;
	returned = 0;
	printf("setting jump label\n");

	retval = setjmp(jump_buffer);
	printf("past label\n");
	if(!returned) {
		printf("returning directly\n");
		return execute1(Head);
	}
	else {
		printf("returning retval\n");
		return retval;
	}

	return retval;
}
