#!/usr/bin/fish

set iters 100 10000 1000000 10000000
set gc_iters 100 1000 10000 20000 30000

echo ">  Benchmarking arena"
for i in $iters
    set r (/usr/bin/time -f "%e" ./arena_vs_stock arena $i 2>&1)
    echo "$i $r" >> bm_arena.txt
end

echo ">  Benchmarking malloc"
for i in $iters
    set r (/usr/bin/time -f "%e" ./arena_vs_stock malloc $i 2>&1)
    echo "$i $r" >> bm_malloc.txt
end


echo ">  Benchmarking gc"
for i in $gc_iters
    set r (/usr/bin/time -f "%e" ./arena_vs_stock gc $i 2>&1)
    echo "$i $r" >> bm_gc.txt
end
