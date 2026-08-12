# HCH - Handy C Headers

## WARNING!
**I DON'T TAKE ANY RESPONSIBILITY FOR ANYTHING THAT CAN GO WRONG WHEN USING MY MODE, SO USE IT AT YOUR OWN RISK AND ONLY IF YOU KNOW WHAT YOU ARE DOING.**
**THIS HEADERS CAN CHANGE AT ANY POINT IN TIME WITHOUT WARNING AS I AM STILL DEVELOPING MY TOOLKIT. NO WARRANTY IS GIVEN!**

## About
Single header file libraries (`snippets`) written in C for my personal projects.
Snippets meant to be highly reusable and easy do distribute and package by simply concatenating them.
C89 is desired target of all of these headers for compatibility reasons, but API for C99 is provided for 
less tedious programming experience.


## Build system (Tsoding advertisement.)
This projects uses NoBuild (nob) project made by Tsoding (Alexey).
I use it because I like it, and also because it doesn't have any apparent downsides over
something like `Make`, `Cmake` and Shell scripts.

Link: [NOB.H](https://github.com/tsoding/nob.h)

## Overview:

The goal of this project is to provide to myself (or random users):

- Self contained headers in C89 that enforce sane programming practices.
- Convenient Data structures and algorithms.
- Stuff C doesn't have: (Proper safe variadics, Generics, Runtime reflections, Compile-time code generation features, etc..).
- Self-documented code.
- Easily cross-reusable between different languages (D-lang, Jai, Odin?, Zig??, etc.). 

## Content:

Interface struct's:

- [x] Allocator interface - a easy-to-use, easy-to-change memory allocation.
- [x] Generic interface - a way to express (self-document) passing pointers with sizes as "Generic types".
- [ ] Sink interface - for writing into: files, buffers, pipes, etc.
- [ ] Graphics/Canvas interface - for drawing without thinking what graphics provider is used.

Headers:

- [x] hc_memory.h - Memory utility functions and Allocator Interface reference implementation.
- [x] hc_bump.h - Bump allocator.
- [x] hc_arena.h - Arena allocator.
- [x] hc_generic.h - Generic interface reference implementation.
- [x] hc_array.h - Resizable array data structure.
- [x] hc_xarray.h - Resizable exponential array data structure.
- [x] hc_table.h - Hash table.
- [x] hc_grid.h - 2D grid header for Graphical contexts.
- [x] hc_pool.h, hc_pool2.h - Two different implementations of Pool data structure, they differ in approach to storing free-list indexes.
- [x] hc_string.h - Implementation of String, CString, MutableCString, StringBuilder types and their functions.
- [x] hc_bitset.h - Bitset (Bit array).
- [x] hc_args.h - Command Line Arguments parsing. (**SUBJECT TO TOTAL CHANGE**).
- [x] hc_tokenize.h - Context-less tokenizer functions.
- [ ] hc_comptime.h - Compile time functionality in C. Code seekers, matchers, in-place changers, generators.
- [ ] hc_datalan.h - Data Language similar to JSON but less bad.
- [ ] hc_math.h - All of the math functionality like Linear Algebra, Geometry, and other.
- [ ] hc_list.h - List and Tree data structure types and macros.
- [ ] hc_unrollist.h - Implementation of Unrolled List container for any type.
- [ ] hc_time.h - Implementation of Time types and profiler.
- [ ] hc_graphics.h - Reference implementation of graphics interface.
- [ ] hc_sset.h - Implementation of Sparse Set.
- [ ] hc_relational_storage.h - Implementation of ECS-like fast, memory efficient, archetype storage for general use in optimized software (such as games or whatnot).
- ...More? I might add anything really, maybe later...

## Bindings:

So far no bindings, but I'm planning on using my headers in `D`-lang, so when that time comes, there will be `bindings` directory.

All bindings will have an instruction on how to use them and an example of build script for a project!
