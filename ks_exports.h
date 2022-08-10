#ifndef KS_EXPORTS_H_
#define KS_EXPORTS_H_

#include "ks_mutual.h"

char* compile(char* name, int parameter_count, char** parameter_names, int* parameter_types, char* script);
char* run_function(char* name, symval* parameter_values);
char* eval(char* script);

char const * const * get_function_parameters(char const * function_name, int* count);


#endif /* KS_EXPORTS_H_ */
