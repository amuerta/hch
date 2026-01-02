# HCH - Handy C Headers

## About
Single header file libraries (i call them `snippets`) written in C for fun or a need. 
Snippets meant to be highly reusable and easy do distribute and package by simply concatenating them.



## WARNING
**I DON'T TAKE ANY RESPONSIBILITY FOR ANYTHING THAT CAN GO WRONG WITH MY CODE, SO USE IT AT YOUR OWN RISK AND ONLY IF YOU KNOW WHAT YOU ARE DOING.**

## AD
[NOB.H](https://github.com/tsoding/nob.h)

## Overview:

The goal of this "snippets" is to:

- be self contained, in such way that you can package them by just `cat`-ing them into whatever suits you.
- be self documented, you can just read the source.
- suit my needs, (which means)
  + easy to use
  + minimal setup and memory overhead 
  + configured easily
  + you can just compile with single define 
  + and have a static library you can link with whatever can interface C

In hc.h i have:
- C QOL macros
- types (like s32, u32, mstr, ssize, etc)
- dynamic array (tsoding style)
- link stuff (link as linked list and tree links)
- arena allocator
- temporary allocator
- simple profiler
- free list
- immutable string
- string builder (nob.h style)
- map (double hashing + open indexing)
- arg parsing
- pool data structure

Outside of the hc.h (things i haven't decided fate of with hc.h)

- tokenizer.h
- sparse_set.h

## Whishlist

Things i wanna do for hc.h or just as stand alone header in this repo

- stack allocator using virtually mapped memory
- tree node operations like in linked lists li_append_children()
- format.h - for colored output in both ANSI compliant and Win32 enviroments
- md.h  - markdown parser
- expr.h - simple expression evaluator for math and variables.
