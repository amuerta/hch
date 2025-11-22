arena: arena-build

benchmark-arena:
	./benchmark.sh

arena-build:
	clang -o arena_vs_stock arena_vs_stock.c -ggdb -L=./external/gc/src -L=./external/gc/build/src

	
arena-run:
	./arena_vs_stock arena
