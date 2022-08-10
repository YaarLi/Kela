#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ks_internal.h"

TreeNode* mkleaf(int type, symval value) {
  TreeNode* p = (TreeNode*) malloc(sizeof(TreeNode));
  p->type = type;
  p->leaf_value = value;
  return p;
}

TreeNode*
mknode(int type, TreeNode* a0, TreeNode* a1, TreeNode* a2) {
  TreeNode* p = (TreeNode*) malloc(sizeof(TreeNode));
  p->type = type;
  p->args[0] = a0;
  p->args[1] = a1;
  p->args[2] = a2;
  return p;
}

int lookahead;

void match(int token){
	if(lookahead == token){
		lookahead = lexical();
	} else {
		char errmsg[] = "syntax error: unexpected token X";
		errmsg[strlen(errmsg)-1] = lookahead;
		comp_err(errmsg);
	}
}

TreeNode *list(), *assignment(), *block(), *factorf(), *factori();


#define defmaths(type) \
TreeNode *expression##type (), *expression1##type (TreeNode*), *term##type (), *term1##type (TreeNode*);\
\
TreeNode* expression##type (){\
	TreeNode* op1 = term##type ();\
	return expression1##type (op1);\
}\
\
TreeNode* expression1##type (TreeNode* op1){\
	TreeNode *op2, *N;\
	int op;\
	switch(lookahead){\
		case '+':\
		case '-':\
		case '<':\
		case '>':\
			op = lookahead;\
			match(lookahead);\
			op2 = term##type ();\
			N = mknode(op, op1, op2, NULL);\
			return expression1##type (N);\
	}\
	return op1;\
}\
\
TreeNode* term##type (){\
	TreeNode* op1 = factor##type ();\
	return term1##type (op1);\
}\
\
TreeNode* term1##type (TreeNode* op1){\
	TreeNode* op2;\
	int op = lookahead;\
	switch(lookahead){\
	case '*':\
	case '/':\
		match(lookahead);\
		op2 = factor##type ();\
		return term1##type (mknode(op, op1, op2, NULL));\
	}\
	return op1;\
}

defmaths(f)
defmaths(i)


TreeNode* factorf (){\
	TreeNode* N;
	symval TV = cur_token_val;
	printf("lookahead: %c/%d\n", lookahead, lookahead);
	switch(lookahead){
	case IDINT:
	    match(IDINT);
		return mkleaf(IDINT, TV);
	case IDFLOAT:
		match(IDFLOAT);
		return mkleaf(IDFLOAT, TV);	
	case INT:
		match(INT);
		TV.floating = (double) TV.integer;
		return mkleaf(FLOAT, TV);
	case FLOAT:
		match(FLOAT);
		return mkleaf(FLOAT, TV);
	case '(':
		match('(');
		N = expressionf();
		match(')');
		return N;
	}
	printf("did not return in switch\n");
	char errmsg[] = "Unexpected token X, expected factor";
	errmsg[17] = lookahead;
	comp_err(errmsg);
	return NULL; /*should exit before this, but compiler might warn without this line*/ 
}

TreeNode* factori (){\
	TreeNode* N;
	symval TV = cur_token_val;
	printf("lookahead: %c/%d\n", lookahead, lookahead);
	switch(lookahead){
	case IDINT:
	    match(IDINT);
		return mkleaf(IDINT, TV);
	case IDFLOAT:
		match(IDFLOAT);
		return mkleaf(IDFLOAT, TV);	
	case INT:
		match(INT);
		return mkleaf(INT, TV);
	case FLOAT:
		comp_err("Using a float when setting int makes no sense in current syntax");
		//match(FLOAT);
		//TV.integer = (long long) TV.floating;
		return mkleaf(INT, TV);
	case '(':
		match('(');
		N = expressioni();
		match(')');
		return N;
	}
	printf("did not return in switch\n");
	char errmsg[] = "Unexpected token X, expected factor";
	errmsg[17] = lookahead;
	comp_err(errmsg);
	return NULL; /*should exit before this, but compiler might warn without this line*/ 
}

TreeNode* parse(){
	lookahead = lexical();
	TreeNode* N = list();
	match(DONE);
	return N;
}

TreeNode* block(){
	match('{');
	TreeNode* L = list();
	match('}');
	return L;
}

TreeNode* list(){
	TreeNode *A, *B, *C;
	//int L;
	switch(lookahead){
	case IDINT:
	case IDFLOAT:
	case TYPE:
		A = assignment();
		match(';');
		B = list();
		if(B){
			return mknode(';', A, B, NULL);
		}
		return A;
	case IF:
		//L = lookahead;
		match(IF); match('(');
		A = expressionf();
		match(')');
		B = block();
		if(lookahead == ELSE){
			match(ELSE);
			C = block();
		} else {
			C = mkleaf(0, (symval) (long long) 0); //do nothing node
		}
		return mknode(';', mknode(IF, A, B, C), list(), NULL);
	case WHILE:
		//L = lookahead;
		match(WHILE); match('(');
		A = expressionf();
		match(')');
		B = block();
		C = list();
		return mknode(WHILE, A, B, C);
	case RETURN:
		match(RETURN);
		A = expressionf();
		match(';');
		return mknode(RETURN, A, NULL, NULL);
	default:
		return NULL;
	}
}

TreeNode* assignment(){
	if(lookahead == TYPE) match(TYPE);
	if(symbols[cur_token_val.integer].val_type == IDINT){
		TreeNode* lval = mkleaf(IDINT, cur_token_val);
		match(IDINT); match('=');
		//comp_err("int is temporarily not implemented");
		TreeNode* rval = expressioni();
		return mknode(SETINT, lval, rval, NULL);
	}
	else if(symbols[cur_token_val.integer].val_type == IDFLOAT){
		TreeNode* lval = mkleaf(IDFLOAT, cur_token_val);
		match(IDFLOAT); match('=');
		TreeNode* rval = expressionf();
		return mknode(SETFLOAT, lval, rval, NULL);
	} else {
		comp_err("unrecognized type");
	}
}
