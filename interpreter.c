#include <stdio.h>
#include "ks_internal.h"

int execute(TreeNode* N){
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
  	    return execute(N->args[0]) + execute(N->args[1]);
  	case '-':
  	    return execute(N->args[0]) - execute(N->args[1]);
  	case '*':
  	    return execute(N->args[0]) * execute(N->args[1]);
  	case '/':
  	    return execute(N->args[0]) / execute(N->args[1]);
  	case '%':
  	    return execute(N->args[0]) % execute(N->args[1]);
  	case '=':
  	    R = execute(N->args[1]);
  	    symbols[N->args[0]->leaf_value].value = R;
  	    return R;
  	case '<':
  	    return (int) (execute(N->args[0]) < execute(N->args[1]));
  	case '>':
  	    return (int) (execute(N->args[0]) > execute(N->args[1]));
  	case ';':
  		if(N->args[0]->type == RETURN) return execute(N->args[0]);
  		execute(N->args[0]);
        return execute(N->args[1]);
  	case WHILE:
		while(execute(N->args[0])) execute(N->args[1]);
		return execute(N->args[2]);
	case IF:
		if(execute(N->args[0])) execute(N->args[1]);
		return execute(N->args[2]);
	case RETURN:
		return execute(N->args[0]);
  }
  return 0;
}
