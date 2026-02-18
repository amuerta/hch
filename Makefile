run: tracker

run-all: mem tracker arr link_tree list_and_arena sb map pool args arena string

loc: 
	wc -lc ./src/*.h

build-header:
	cat ./src/*.h > ./packaged/hc.h

tracker:
	cc -o ./examples/exec/memtracker ./examples/memtracker.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/memtracker

link_tree:
	cc -o ./examples/exec/tree ./examples/tree.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/tree

string:
	cc -o ./examples/exec/string ./examples/string.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/string

bitmasking:
	cc -o ./examples/exec/bitmasking ./examples/bitmasking.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/bitmasking

mem:
	cc -o ./examples/exec/memory ./examples/memory.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/memory

arr:
	cc -o ./examples/exec/array ./examples/array.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/array

trie:
	cc -o ./examples/exec/trie ./examples/trie.c -ggdb -pg -Wextra -Wall
	./examples/exec/trie

list_and_arena:
	cc -o ./examples/exec/list_and_arena ./examples/list_and_arena.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/list_and_arena

sb:
	cc -o ./examples/exec/sb ./examples/sb.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/sb

arena:
	cc -o ./examples/exec/arena ./examples/arena.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/arena

pool:
	cc -o ./examples/exec/pool ./examples/pool.c -ggdb -pg -Wextra -Wall
	./examples/exec/pool

set:
	cc -o ./examples/exec/sset ./examples/sset.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/sset

map:
	cc -o ./examples/exec/map ./examples/map.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/map

map_counting:
	cc -o ./examples/exec/map_counting ./examples/map_counting.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/map_counting

args:
	cc -o ./examples/exec/args ./examples/args.c -ggdb -pg -Wextra -Wall
	./examples/exec/args
