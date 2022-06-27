#pragma once

#include <wchar.h>

#define STRINGIFY(s) #s

#define LEX_LEN_MAX 128

extern char const * script_string;
extern int script_char_ind;
extern int line_ind;
extern int cur_token_val;
extern int declaration_type;

#define MAX_SYMBOL_COUNT 100

struct symbol { 
    char *lexeme; 
    int sym_type;
    int val_type;
    int value;
};
//extern struct symbol symbols[];
extern struct symbol* symbols;
void initialise_symbols();

int lookup(char*);
int insert(char* lexeme, int type);

//lexeme types
#define INT    256
#define ID     259
#define TYPE   260
#define WHILE  261
#define IF     262
#define ELSE   263
#define RETURN 264
#define DONE   265

int lexical();

void comp_err(char* message);


#define MAX_ARGS 3
typedef struct TN TreeNode;
struct TN {
  int type;
  int leaf_value;
  TreeNode* args[MAX_ARGS];

};

TreeNode* parse();

void printtree(TreeNode* p);
int execute(TreeNode* N);

#define STACK_SIZE 2000
