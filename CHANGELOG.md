# CHANGELOG 

Should match the commit history, or at least give you an idea of what changed happened and when.

# V0
**I didn't track changes, view commit history.**

![Silly me](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fmedia.tenor.com%2FT-LIt12mhjIAAAAM%2Fsilly-me.gif&f=1&nofb=1&ipt=750cd1bcfac40fd28a4931165d112ff2dfa9a86530892df1660d2603694c2eca)


# V1 - Since Wed Aug 12 04:50 PM EEST 2026
0. Reworked project structure. Now all headers are in `./src/headers` and all examples are in `./src/examples`. Switch build system from `make` to `nob.h`. Added `CHANGELOG.md`. Changed `README.md`. More changes that just delete or move files, not important.
0. Added comma-operator expression trick to array append API. Added a macro to opt-out of this.
0. Fixed a bug in `hc_array_resize_generic()` that had USE AFTER FREE. Still need to rewrite this function... Eventually.
0. Build script now has `--test` command that runs a specific example with all precautions.
0. Added `hc_` prefix to where it was missing.
0. Removed old `./examples`.
0. Added missing `--nerd` flag response for `./build.c`.
0. Updated `array.c` to have use of `hc_array_unordered_remove()`.
