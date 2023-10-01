build:
	gcc -std=c99 -Wall ./lib/lua/src/*.c ./src/main.c -o main -I /opt/homebrew/Cellar/sdl2/2.28.3/include -L /opt/homebrew/lib/ -lSDL2 -lm
	gcc -std=c99 -Wall ./lib/lua/src/*.c ./src/lua_examples.c -o examples -lm
	g++ -std=c++17 ./src/table_parser.cpp -o parser -llua5.4
	gcc -o read_table ./src/read_table.c -I /opt/homebrew/Cellar/lua/5.4.6/include/lua -L /opt/homebrew/Cellar/lua/5.4.6/lib -llua -lc
clean:
	rm ./main ./examples parser

