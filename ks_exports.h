#ifndef KS_EXPORTS_H_
#define KS_EXPORTS_H_

#define INT    256

char* compile(char* name, int parameter_count, char** parameter_names, int* parameter_types, char* script);
char* run_function(char* name, int* parameter_values);

char const * const * get_function_parameters(char const * function_name, int* count);


#endif /* KS_EXPORTS_H_ */
