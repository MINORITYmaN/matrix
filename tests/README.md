# Allocation regression tests

## Portable logic

Run `python -m pytest -q` with pytest and a C99 compiler (`CC`, default `cc`).
The harness extracts CreateMatrix, DestroyMatrix, the matrix declarations and the
WM_NCCREATE/WM_NCDESTROY cases directly from production sources. No production
allocation logic is copied into the test. GDI/window calls are stubbed here;
these checks do not establish native Windows behavior.

The allocator returns NULL for the first, middle and last of five column
allocations, and separately for the outer matrix. Allocated memory starts with
nonzero bytes to verify initialization. Each failure must stop allocation,
return NULL, release every earlier allocation exactly once, and acquire no DC.
The success case verifies zeroed glyphs and complete destruction, including
clearing the caller's pointer. Window-message checks verify that failure neither
publishes user data nor starts a timer, and subsequent destruction does not
free anything twice.

Optional sanitizer run: compile the generated harness with
`-fsanitize=address,undefined`. Some container environments require
`ASAN_OPTIONS=detect_leaks=0`; the harness independently tracks every allocation
and rejects leaks, invalid frees and double frees on the tested paths.

## Native Windows lifecycle

From an MSVC Developer Command Prompt, run:

```
python tests/windows/run_allocation.py
```

This separate executable uses real User32/GDI calls. It registers a window class
using the production creation/destruction message cases, then injects each of
the three column failures during CreateWindowExW. Creation must return NULL,
release all memory, and start no timer. A successful creation/destruction pair
must publish matrix user data, start a timer, and free all allocations.
MakeBitmap is replaced with a compatible bitmap; this is not a rendering test
or a complete application build. No tests modify animation/rendering functions.

Before merging, build the full application on Windows and check fullscreen,
`/p` and settings previews, including ordinary exit. The Windows executable and
these manual application checks were not run in the Linux authoring environment.
