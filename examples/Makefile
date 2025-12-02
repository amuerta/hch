run: string

run-all: da list_and_arena sb maps pol args arena string

build-header:
	cat ../src/prelude.h > ../hc.h
	cat ../src/da.h >> ../hc.h
	cat ../src/sb.h >> ../hc.h
	cat ../src/link.h >> ../hc.h
	cat ../src/arena.h >> ../hc.h
	cat ../src/args.h >> ../hc.h
	cat ../src/string.h >> ../hc.h
	cat ../src/maps.h >> ../hc.h
	cat ../src/pool.h >> ../hc.h


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

maps:
	cc -o ./exec/maps maps.c -ggdb -pg -Wextra -Wall -fsanitize=address
	./exec/maps

args:
	cc -o ./exec/args args.c -ggdb -pg -Wextra -Wall
	./exec/args
