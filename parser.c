#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ks_internal.h"

TreeNode* mkleaf(int type, int value) {
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

TreeNode *list(), *assignment(), *expression(), *expression1(TreeNode*), *term(), *term1(TreeNode*), *factor(), *block();

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
	int L;
	switch(lookahead){
	case ID:
	case TYPE:
		A = assignment();
		match(';');
		B = list();
		if(B){
			return mknode(';', A, B, NULL);
		}
		return A;
	case IF:
	case WHILE:
		L = lookahead;
		match(L); match('(');
		A = expression();
		match(')');
		B = block();
		C = list();
		return mknode(L, A, B, C);
	case RETURN:
		match(RETURN);
		A = expression();
		match(';');
		return mknode(RETURN, A, NULL, NULL);
	default:
		return NULL;
	}
}

TreeNode* assignment(){
	//printf("assign.\n");
	if(lookahead == TYPE) match(TYPE);
	TreeNode* lval = mkleaf(ID, cur_token_val);
	match(ID); match('=');
	TreeNode* rval = expression();
	return mknode('=', lval, rval, NULL);
}

TreeNode* expression(){
	//printf("expr.\n");
	TreeNode* op1 = term();
	return expression1(op1);
}


TreeNode* expression1(TreeNode* op1){
	TreeNode *op2, *N;
	int op;
	switch(lookahead){
		case '+':
		case '-':
		case '<':
		case '>':
			op = lookahead;
			match(lookahead);
			op2 = term();
			N = mknode(op, op1, op2, NULL);
			return expression1(N);
	}
	return op1;
}

TreeNode* term(){
	//printf("term\n");
	TreeNode* op1 = factor();
	return term1(op1);
}

TreeNode* term1(TreeNode* op1){
	TreeNode* op2;
	int op = lookahead;
	switch(lookahead){
	case '*':
	case '/':
		match(lookahead);
		op2 = factor();
		return term1(mknode(op, op1, op2, NULL));
	}
	return op1;
}

TreeNode* factor(){
	TreeNode* N;
	int TV = cur_token_val;
	switch(lookahead){
	case ID:
	    match(ID);
		return mkleaf(ID, TV);
	case INT:
		match(INT);
		return mkleaf(INT, TV);
	case '(':
		match('(');
		N = expression();
		match(')');
		return N;
	}
	char errmsg[] = "Unexpected token X, expected factor";
	errmsg[17] = lookahead;
	comp_err(errmsg);
	return NULL; //should exit before this, but compiler might warn without this line
}
