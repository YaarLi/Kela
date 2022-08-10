#pragma once

#define STRINGIFY(s) #s

#define LEX_LEN_MAX 128

#include "ks_mutual.h"

extern char const * script_string;
extern int script_char_ind;
extern int line_ind;
extern symval cur_token_val;
extern int declaration_type;

#define MAX_SYMBOL_COUNT 100


struct symbol { 
    char *lexeme; 
    int sym_type;
    int val_type;
    symval value;
};
//extern struct symbol symbols[];
extern struct symbol* symbols;
void initialise_symbols();

int lookup(char*);
int insert(char* lexeme, int type);

//lexeme types
#define TYPE   256
#define WHILE  257
#define IF     258
#define ELSE   259
#define RETURN 260

//#define INT    263
//#define IDINT  264
#define ADDINT 265
#define SUBINT 266
#define MULINT 267
#define DIVINT 268
#define MODINT 269
#define GTINT  270
#define LTINT  271
#define SETINT 272

#define ADDFLOAT 273
#define SUBFLOAT 274
#define MULFLOAT 275
#define DIVFLOAT 276
#define GTFLOAT  277
#define LTFLOAT  278
#define SETFLOAT 279
//#define FLOAT  280
//#define IDFLOAT 281

#define DONE   282

/*
inline char returnsint(int lexeme){
	if(lexeme >= INT && lexeme <= FLOAT2INT) return (char) 1;
	return (char) 0;
}

inline char returnsfloat(int lexeme){
	if(lexeme >= ADDFLOAT && lexeme <= IDFLOAT) return (char) 1;
	return (char) 0;
}
*/



int lexical();

void comp_err(char* message);


#define MAX_ARGS 3
typedef struct TN TreeNode;
struct TN {
  int type;
  symval leaf_value;
  TreeNode* args[MAX_ARGS];

};

TreeNode* parse();

void printtree(TreeNode* p);
double execute_tree(TreeNode* N);

#define STACK_SIZE 2000
