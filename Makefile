run: map_counting map

run-all: da list_and_arena sb map pool args arena string

build-header:
	cat ./src/*.h > ./packaged/hc.h

string:
	cc -o ./examples/exec/string ./examples/string.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/string

bitmasking:
	cc -o ./examples/exec/bitmasking ./examples/bitmasking.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/bitmasking

da:
	cc -o ./examples/exec/da ./examples/da.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./examples/exec/da

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
