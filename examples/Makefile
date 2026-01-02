run: list_and_arena

run-all: da list_and_arena sb map pol args arena string

build-header:
	cat ../src/*.h > ../hc.h

string:
	cc -o ./exec/string string.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/string

bitmasking:
	cc -o ./exec/bitmasking bitmasking.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/bitmasking

da:
	cc -o ./exec/da da.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/da

trie:
	cc -o ./exec/trie trie.c -ggdb -pg -Wextra -Wall
	./exec/trie

markdown:
	cc -o ./exec/markdown markdown.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/markdown

list_and_arena:
	cc -o ./exec/list_and_arena list_and_arena.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/list_and_arena

ansi:
	cc -o ./exec/ansi ansi.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/ansi

sb:
	cc -o ./exec/sb sb.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/sb

arena:
	cc -o ./exec/arena arena.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/arena

pol:
	cc -o ./exec/pool pool.c -ggdb -pg -Wextra -Wall
	./exec/pool

set:
	cc -o ./exec/sset sset.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/sset

map:
	cc -o ./exec/map map.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/map

args:
	cc -o ./exec/args args.c -ggdb -pg -Wextra -Wall
	./exec/args
