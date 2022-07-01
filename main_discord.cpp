#include <dpp/dpp.h>
#include <dpp/fmt/format.h>
#include <iostream>
extern "C"{
#include "ks_exports.h"
}
#include <string>
#include <vector>
#include <variant>

char** to_cstring_array(std::string message, int* size){
	bool was_alnum = false;
	std::vector<std::string> strings;
	std::cout << "message: " << message << '\n';
	for(char c : message){
		if(!was_alnum && isalnum(c)){
			std::cout << "1/" << c;
			was_alnum = true;
			strings.push_back(std::string("") + c);
			std::cout << strings.back();
		} else if(was_alnum){
			std::cout << "2/" << c;
			if(isalnum(c)){
				strings.back() += c;
			} else {
				was_alnum = false;
			}
		} else {std::cout << "3//";}
	}
	std::cout << "strings - size " << strings.size();// << " content: " << strings << '\n';
	char** array = (char**) malloc(strings.size()*sizeof(char*));
	int i = 0;
	for(std::string s : strings){
		std::cout << "s: " << s << '\n';
		array[i++] = strdup(s.c_str());
	}
	*size = strings.size();
	return array;
}

void save_command(std::string name, dpp::snowflake guild_id, std::string code, int n_parameters, char** params){
	std::ofstream commands_file;
	commands_file.open("saved_commands.txt", std::ofstream::out | std::ofstream::app);
	if(!commands_file.is_open()){
		std::cout << "Failed to open the saved commands file!\n";
		return;
	}
	commands_file << '\n' << guild_id << '\n' << name << '\n' << n_parameters << '\n';
	for(int i = 0; i < n_parameters; i++){
		commands_file << params[i] << ' ';
	}
	commands_file << '\n' << code.length() << '\n' << code;
	commands_file.close();
}

void load_saved_commands(){
	std::ifstream commands_file("saved_commands.txt", std::ifstream::in);
	if(!commands_file.is_open()){
		std::cout << "Failed to open the saved commands file!\n";
		return;
	}
	try{
	while(commands_file.good()){
		std::cout << "loading command\n";
		dpp::snowflake guild_id;
		commands_file >> guild_id;
		std::string name;;
		commands_file >> name;
		std::cout << "name: " << name << '\n';
		int par_c;
		commands_file >> par_c;
		char** params = (char**) malloc(par_c*sizeof(char*));
		int* partypes = (int*) malloc(par_c*sizeof(int));
		std::cout << "params:";
		for(int i = 0; i < par_c; i++){
			std::string tmp;
			commands_file >> tmp;
			std::cout << ' ' << tmp;
			params[i] = strdup(tmp.c_str());
			partypes[i] = INT;
		}
		int codelen;
		commands_file >> codelen;
		commands_file.ignore(1);
		std::cout << "\ncodelen: " << codelen << '\n';
		char* code = (char*) malloc(codelen+1);
		commands_file.read(code, codelen);
		code[codelen] = '\0';
		commands_file.ignore(1);

		std::stringstream ss;
		ss << guild_id << name;
		std::cout << "code:\n" << code << '\n';
		if(commands_file){
			std::cout << "Successfully read from file\n";
		} else {
			std::cout << "Only read " << commands_file.gcount() << "/" << codelen << '\n';
		}
		char* res = compile(strdup(ss.str().c_str()), par_c, params, partypes, code);
		if(res){
			std::cout << "Epic compilation fail:\n`" << res << '\n';
			return;
		}
	}
	} catch (const std::exception& e){
		std::cout << "Error during loading commands:\n" << e.what() << '\n';
	}

}

int main()
{

	FILE* key_file = fopen("bot_key.txt", "r");
	if(!key_file){
		std::cout << "File with bot key not found!\n";
		return 1;
	}
	char key[100];
	fgets(key, 100, key_file);
    dpp::cluster bot(key);
    fclose(key_file);

    load_saved_commands();

    bot.on_ready([&bot](const dpp::ready_t & event) {
        std::cout << "Logged in as " << bot.me.username << "!\n";

        try{
        if (dpp::run_once<struct register_bot_commands>()) {

            /* Create a new global command on ready event */
            dpp::slashcommand compcommand("compile", "Compile a kelascript command", bot.me.id);

            compcommand.add_option(
                    dpp::command_option(dpp::co_string, "name", "The name of the command", true)
            );
            compcommand.add_option(
					dpp::command_option(dpp::co_string, "int_parameters", "The variable names of integer parameters", true)
			);
            compcommand.add_option(
            		dpp::command_option(dpp::co_string, "code", "The actual code", true)
            );
            compcommand.add_option(
            		dpp::command_option(dpp::co_string, "description", "The description that the user sees for the new slash command", true)
            );

            /* Register the command */
            bot.global_command_create(compcommand);
            bot.guild_command_create(compcommand, 545231706299301888);
            printf("Compile command created\n");
            
            

            dpp::slashcommand evalcommand("eval", "Evaluate code without saving it as command", bot.me.id);
            evalcommand.add_option(
            		dpp::command_option(dpp::co_string, "code", "The code", true)
            );


            /* Register the command */
            bot.global_command_create(evalcommand);
            bot.guild_command_create(evalcommand, 545231706299301888);
            printf("Compile command created\n");
            
        }
        } catch (const std::exception& e){
        	std::cout << e.what();
        }
        printf("Ready!\n");
    });

    bot.on_log(dpp::utility::cout_logger());

    bot.on_interaction_create([&bot](const dpp::interaction_create_t & event) {
    	try {
			std::string commandname = event.command.get_command_name();
			if (commandname == "compile") {
				std::cout << "getting compilation data\n";
				int varcount;
				//auto p = event.get_parameter("name");
				std::string funcname = std::get<std::string>(event.get_parameter("name"));
				std::string intvars = std::get<std::string>(event.get_parameter("int_parameters"));
				std::cout << "setting up variables\n";
				char** varnames = to_cstring_array(intvars, &varcount);
				int* vartypes = (int*) malloc(sizeof(int)*varcount);
				for(int i = 0; i < varcount; i++){vartypes[i] = INT;std::cout << "variable " << varnames[i] << '\n';}
				std::string rawcode = std::get<std::string>(event.get_parameter("code"));
				std::cout << "copying code\n";
				char* code = strdup(rawcode.c_str());
				std::stringstream ss;
				ss << event.command.guild_id << funcname;
				std::cout << "compiling\n";
				char* res = compile(strdup(ss.str().c_str()), varcount, varnames, vartypes, code);
				if(res){
					ss.str("");
					ss << "Epic compilation fail:\n`" << res << '`';
					event.reply(ss.str());
					return;
				}
				std::string description = std::get<std::string>(event.get_parameter("description"));

				std::cout << "registering command\n";
				dpp::slashcommand newcommand(funcname, description, bot.me.id);
				for(int i = 0; i < varcount; i++){
					newcommand.add_option(
							dpp::command_option(dpp::co_integer, varnames[i], varnames[i], true)
					);
				}
				bot.guild_command_create(newcommand, event.command.guild_id);
				save_command(funcname, event.command.guild_id, rawcode, varcount, varnames);
				event.reply(dpp::message("Compilation successful, you should see the new slash command shortly!"));
			} else if(commandname == "eval"){
				std::string rawcode = std::get<std::string>(event.get_parameter("code"));
				char* code = strdup(rawcode.c_str());
				std::cout << "compiling and running\n";
				char* rep = eval(code);
				if(!rep){
					event.reply("Error - eval function call returned NULL. This is a bug.\n");
					return;
				}
				std::stringstream ss;
				ss << '`' << rep << '`';
				event.reply(ss.str());
				
			
			} else {
				std::stringstream ss;
				ss << event.command.guild_id << event.command.get_command_name();
				std::cout << "running command: " << event.command.get_command_name() << '\n';
				char* internal_command_name = strdup(ss.str().c_str());
				int parcount;
				std::cout << "getting parameters...\n";
				char const * const * param_names = get_function_parameters(internal_command_name, &parcount);
				if(!param_names){event.reply(dpp::message("Could not fetch the command")); return;}
				int* param_values = (int*) malloc(parcount*sizeof(int));
				std::cout << "writing " << parcount << " parameter values...\n";
				for(int i = 0; i < parcount; i++){
					std::cout << param_names[i] << ' ';
					param_values[i] = std::get<int64_t>(event.get_parameter(param_names[i]));
					std::cout << param_values[i] << '\n';
				}
				std::cout << "running...\n";
				std::cout << "(the function)\n";
				char* rep = run_function(internal_command_name, param_values);
				event.reply(dpp::message(rep));
			}
    	} catch(const std::exception& e){
    		std::cout << e.what();
    		event.reply(dpp::message("An unhandled error has occured"));
    	}
    });
    bot.start(false);
    return 0;
}
