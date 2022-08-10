#include <wchar.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ks_internal.h"

//struct symbol symbols[MAX_SYMBOL_COUNT];
struct symbol* symbols = NULL;
int symbol_count = 0;


int lookup(char* lexeme){
 for(int i = symbol_count-1; i >= 0; i--){ //most recent occurrence, if duplicates, returns first
     if(!strcmp(lexeme, symbols[i].lexeme)){
         return i;
     }
 }
    return -1;
}

int insert(char* lexeme, int type){
	if(symbol_count > MAX_SYMBOL_COUNT){
		comp_err("Too many symbols");
	}
	symbols[symbol_count].sym_type = type;
	symbols[symbol_count].lexeme = strdup(lexeme);
	symbols[symbol_count].val_type = declaration_type;
	declaration_type = 0;
	return symbol_count++;
}

void initialise_symbols(){
	symbol_count = 0;
	int s = insert("int", TYPE);
	symbols[s].value.integer = IDINT;
	insert("while", WHILE);
	insert("if", IF);
	insert("else", ELSE);
	insert("return", RETURN);
	s = insert("float", TYPE);
	symbols[s].value.integer = IDFLOAT;
}

char lexeme[LEX_LEN_MAX+1];
int line_ind = 1;
symval cur_token_val = {.integer = -1};
int declaration_type = 0;

int lexical(){
 char c;
 while(1){
  c = script_string[script_char_ind++];
  printf("%d %c\n", (int) c,  c);
  switch(c) {
      case ' ':
      case '\t':
          break;
      case '\n':
          line_ind++;
          break;
      case ';':
      case '=':
      case '+':
      case '-':
      case '*':
      case '/':
      case '(':
      case ')':
      case '{':
      case '}':
      case '<':
      case '>':
    	  cur_token_val.integer = (int) c;
    	  return c;
      case '\0':
      case EOF:
    	  script_char_ind--;
          return DONE;
      default:
        if(isdigit(c)){
            int scanned;
            sscanf(script_string+script_char_ind-1, "%lld%n", &cur_token_val.integer, &scanned);
            if(script_string[script_char_ind-1+scanned] == '.'){
				sscanf(script_string+script_char_ind-1, "%lf%n", &cur_token_val.floating, &scanned);
				script_char_ind+=scanned-1;
				return FLOAT;
			}
            script_char_ind+=scanned-1;
            //printf("scanned %d: %d char", cur_token_val, scanned);
            return INT;
        } else if(isalpha(c)){
            int id_num;
            int lex_len = 0;
            while(isalnum(c)){
                lexeme[lex_len++] = c;
                if(lex_len > LEX_LEN_MAX){
                    comp_err("Lexical error: id length too long, max length is " STRINGIFY(LEX_LEN_MAX));
                }
                c = script_string[script_char_ind++];
            }
            lexeme[lex_len] = '\0';
            //printf("%s\n", lexeme);
            script_char_ind--;
            if(declaration_type){ //if the ID follows a declaration
            	id_num = insert(lexeme, declaration_type);
            } else {
            	id_num = lookup(lexeme);
            }
            if(id_num < 0){
            		char msg[LEX_LEN_MAX+strlen("Error: ID _ is undeclared")+1];
            		snprintf(msg, LEX_LEN_MAX+strlen("Error: ID _ is undeclared"),
            				"Error: ID %s is undeclared", lexeme);
            		comp_err(msg);  //this might be a syntax error,
            						//might be prettier to find somewhere else to do this
            }
            int t = symbols[id_num].sym_type;
            if(t == TYPE){
            	cur_token_val.integer = symbols[id_num].value.integer;
            	declaration_type = cur_token_val.integer;
            	return t;
            }
            cur_token_val.integer = id_num;
            return t;
        }
    //return c;
  }
 }
    
}
