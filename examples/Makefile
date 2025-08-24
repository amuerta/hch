run: sb

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
