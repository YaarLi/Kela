#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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
	if(count){*count = func->parameter_count;}
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
		int j = insert(parnames[i], partypes[i]);
		symbols[j].val_type = partypes[i];
		printf("%d %s %lld %lf\n", j, symbols[j].lexeme, symbols[j].value.integer, symbols[j].value.floating);
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
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	printf("run thread\n");
	kela_function* func = get_function(funcname);
	if(!func) {printf("function not found\n"); return 0;}
	printf("function is found\n");
	symbols = func->symbols;
	printf("set symbols\n");
	for(int i = 0; i < func->parameter_count; i++){
		printf("looking up\n");
		int j = lookup(func->parameters[i]);
		if(j < 0){
			printf("variable not found!\n");
			continue;
		}
		printf("setting value\n");
		symbols[j].value = ((symval*) argv)[i];
		printf("%d %s %lld %lf\n", j, symbols[j].lexeme, symbols[j].value.integer, symbols[j].value.floating);
		
	}
	if(!func->tree){printf("function has no tree!\n"); return 0;}
	double* ret = malloc(sizeof(double));
	*ret = execute_tree(func->tree);
	return (void*) ret;
}

void* kela_eval(void* args){
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	kela_function kelfun;
	symbols = kelfun.symbols;
	initialise_symbols();
	kelfun.name = strdup("tmp");
	kelfun.parameter_count = parcount;
	kelfun.tree = parse();
	if(!kelfun.tree){
		printf("parse returned NULL!\n");
		return strdup("Compilation NULL error!\n");
	}
	
	double ret = execute_tree(kelfun.tree);
	
	char* rep = malloc(100);
	rep[99] = '\0'; //just in case something goes wrong next step
	snprintf(rep, 99, "Call returned: %lf", ret);
	return rep;

}

void* timeout_thread(void* main_thread){
	printf("timeout thread start\n");
	sleep(2);
	printf("timeout!\n");
	printf("cancelled (%d).\n", pthread_cancel((pthread_t) main_thread));
}

pthread_mutex_t uses_global_variables;

char* eval(char* script){
	pthread_mutex_lock(&uses_global_variables);
	script_string = script;
	script_char_ind = 0;
	parcount = 0;
	pthread_t mainthread;
	pthread_t timethread;
	int res = pthread_create(&mainthread, NULL, kela_eval, NULL);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Failed to initiate compilation");
	}
	pthread_create(&timethread, NULL, timeout_thread, (void*) mainthread);
		//printf("failed to create compilation thread! (%d)\n", res); return res;}
	char* rep = NULL;
	res = pthread_join(mainthread, (void**) &rep);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Unhandled error during compilation");
	}
	pthread_mutex_unlock(&uses_global_variables);
	if((void*) rep == PTHREAD_CANCELED) return strdup("Execution timed out");
	printf("execution success!\n");
	pthread_cancel(timethread);
	return rep;
	
}

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

char* run_function(char* name, symval* parameter_values){
	pthread_mutex_lock(&uses_global_variables);
	funcname = strdup(name);
	pthread_t mainthread;
	pthread_t timethread;
	printf("creating thread...");
	int res = pthread_create(&mainthread, NULL, kela_run, (void*) parameter_values);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Failed to initiate execution");
	}
	pthread_create(&timethread, NULL, timeout_thread, (void*) mainthread);
	double* ret;
	res = pthread_join(mainthread, (void**) &ret);
	if(res){
		pthread_mutex_unlock(&uses_global_variables);
		return strdup("Unhandled error during execution");
	}
	pthread_mutex_unlock(&uses_global_variables);
	if((void*) ret == PTHREAD_CANCELED) return strdup("Execution timed out");
	char* rep = malloc(100);
	rep[99] = '\0'; //just in case something goes wrong next step
	snprintf(rep, 99, "Call returned: %lf", *ret);
	free(ret);
	return rep;
}
