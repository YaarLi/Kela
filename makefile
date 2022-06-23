kela_bot: main_discord.cpp kelascript.a ks_exports.h
	g++ -std=c++17 -o kela_bot -pthread  main_discord.cpp kelascript.a -ldpp
	 
kelascript.a: obs ks_exports.h
	ar cr kelascript.a error.o lexer.o parser.o interpreter.o ks_exports.o
	
obs: error.c lexer.c parser.c interpreter.c ks_internal.h ks_exports.c ks_exports.h
	gcc -c error.c lexer.c parser.c interpreter.c ks_exports.c
	
clean:
	rm main kelascript.a kela_bot