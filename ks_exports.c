#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ks_exports.h"
#include "ks_internal.h"

char const * script_string;
int script_char_ind = 0;

struct kelfun {
	char* name;
	TreeNode* tree;
	int parameter_count;
	char** parameters;
	struct symbol symbols[MAX_SYMBOL_COUNT];
};
typedef struct kelfun kela_function;
#define KELA_FUNCTIONS_MAX 50
kela_function kfuncs[KELA_FUNCTIONS_MAX];
int kela_function_count = 0;

char* funcname;
int parcount;
char** parnames;
int* partypes;

kela_function* get_function(char const * function_name){
	for(int i = 0; i < kela_function_count; i++){
		if(!strcmp(kfuncs[i].name, function_name)){
			return &kfuncs[i];
		}
	}

	return NULL;
}

char const * const * get_function_parameters(char const * function_name, int* count){
	kela_function* func = get_function(function_name);
	if(!func) return NULL;
	if(count){*count = parcount;}
	return (char const * const *) func->parameters;
}

void* kela_compile(void* args){
	kela_function* result = &kfuncs[kela_function_count++];
	symbols = result->symbols;
	initialise_symbols();
	result->name = strdup(funcname);
	result->parameters = malloc(parcount*sizeof(char*));
	for(int i = 0; i < parcount; i++){
		result->parameters[i] = strdup(parnames[i]);
		int j = insert(parnames[i], ID);
		symbols[j].val_type = partypes[i];
		printf("%d %s %d\n", j, symbols[j].lexeme, symbols[j].value);
	}
	result->parameter_count = parcount;
	result->tree = parse();
	if(!result->tree){
		printf("parse returned NULL!\n");
		kela_function_count--;
		return strdup("Compilation NULL error!\n");
	}
	return NULL;
}

void* kela_run(void* argv){
	printf("run thread\n");
	kela_function* func = get_function(funcname);
	if(!func) {printf("function not found\n"); return 0;}
	printf("function is found\n");
	symbols = func->symbols;
	printf("set symbols\n");
	for(int i = 0; i < func->parameter_count; i++){
		printf("looking up\n");
		int j = lookup(func->parameters[i]);
		printf("setting value\n");
		symbols[j].value = ((int*) argv)[i];
		printf("%d %s %d\n", j, symbols[j].lexeme, symbols[j].value);
	}
	if(!func->tree){printf("function has no tree!\n"); return 0;}
	int ret = execute(func->tree);
	return (void*) (long) ret;
}

pthread_mutex_t uses_global_variables;

char* compile(char* name, int parameter_count, char** parameter_names, int* parameter_types, char* script){
	pthread_mutex_lock(&uses_global_variables);
	script_char_ind = 0;
	funcname = name;
	parcount = parameter_count;
	parnames = parameter_names;
	partypes = parameter_types;
	script_string = script;
	pthread_t thread;
	int res = pthread_create(&thread, NULL, kela_compile, NULL);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Failed to initiate compilation");
	}
		//printf("failed to create compilation thread! (%d)\n", res); return res;}
	char* errmsg = NULL;
	res = pthread_join(thread, (void**) &errmsg);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Unhandled error during compilation");
	}
		//printf("failed to join compilation thread! (%d)\n", res); return res;}
	pthread_mutex_unlock(&uses_global_variables);
	return errmsg; //NULL if success
}

char* run_function(char* name, int* parameter_values){
	pthread_mutex_lock(&uses_global_variables);
	funcname = strdup(name);
	pthread_t thread;
	int res = pthread_create(&thread, NULL, kela_run, (void*) parameter_values);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Failed to initiate execution");
	}
	int ret;
	res = pthread_join(thread, (void**) &ret);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Unhandled error during execution");
	}
	char* rep = malloc(100);
	rep[99] = '\0'; //just in case something goes wrong next step
	snprintf(rep, 99, "Call returned: %d", ret);
	pthread_mutex_unlock(&uses_global_variables);
	return rep;
}
