# HCH - Handy C Headers

## About
This is collection of data-structures, types, code, api, etc i commonly write on repeat, to follow "good" programming "practices" such as
D.R.Y. (don't reapeat yourself), i put everything that i might want/need in my projects into single "Handy C header" - `hc.h`

## WARNING
**I DONT TAKE ANY RESPONISBILITY FOR ANYTHING THAT CAN GO WRONG WITH MY CODE, SO USE IT AT YOUR OWN RISK.**

## AD
[NOB.H](https://github.com/tsoding/nob.h)

## Overview:
In hc.h i have:

- types (like s32, u32, mstr, ssize, etc)
- dynamic array (tsoding style)
- link stuff (linked list + (maybe trees) - similar to `da_append`)
- arena allocator
- string builder (nob.h style)
- string map (double hashing + open indexing)
- string
- args 
- pool data structure

Outside of the hc.h (things i haven't decided fate of with hc.h)

- tokenizer.h
- sparse_set.h

## Whishlist

Things i wanna do for hc.h or just as stand alone header in this repo

- stack - append elements of dynamic size onto a "stack"
- tree node operations like in linked lists li_append_children()
- format.h - for colored output in both ANSI compliant and Win32 enviroments
- profile.h - simple clock struck for measuring performance in scopes
- md.h  - markdown parser
- expr.h - simple expression evaluator for math and variables (like ini)
