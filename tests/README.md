# Animation regression tests

## Portable logic (Linux/macOS, C99 compiler + Python/pytest)

Run from the repository root:

```sh
python -m pytest -q
```

`test_animation.py` compiles the actual `UpdateAnimationHue` and
`AnimationTimerProc` functions extracted from `src/main.c`. Only the clock,
random generator and renderer are replaced by deterministic recording stubs;
the algorithm is not reimplemented in Python. Simulated timer dispatch covers
1 through 8 windows, 1024 smooth frames (multiple wraps), fixed color/live
settings changes, random jumps and holds, stale timer IDs, stopped timers and
removal of render targets. Every target must receive the same hue once per
shared callback; the number of targets cannot multiply hue or RNG updates.
A source wiring check guards against reintroducing per-window timers.

The expected interval remains `((SPEED_MAX - config.speed) + SPEED_MIN) * 10`
ms, captured when the first window starts, as before. Smooth mode advances one
step per delivered animation callback. A delayed/coalesced Win32 timer does not
catch up elapsed frames. More monitors can increase rendering work and slow
frames on an overloaded machine, but cannot cause extra hue advances.
Random jump mode retains the existing odd-tick selection/even-tick hold rule;
random colors themselves are not expected to reproduce across monitor counts
because glyph animation also uses the application's RNG.

## Windows message-loop integration (Windows + MSVC + Python)

From a Visual Studio Developer Command Prompt in the repository root:

```bat
python tests/windows/run_animation.py
```

This uses the same production functions with real `SetTimer(NULL, ...)`,
`PeekMessage`, `DispatchMessage` and `KillTimer`. Rendering is still recorded,
not drawn. It checks ten real callbacks for 1 through 8 simulated render targets.
It does not require physical monitors and does not validate GDI/window creation.
Do not report this suite as run merely because the portable suite passed.

## Windows application/visual acceptance (separate, manual)

Build the existing `matrix.sln` with the project's Windows SDK/routine/builder
prerequisites. Record Windows version, build configuration and monitor count.
Repeat on 1, 2 and 3+ physical or virtual displays:

1. Set a fixed hue (also try 1 and 255), run `matrix.scr /s`, and confirm every
   display uses it. Exit using the configured keyboard/mouse policy.
2. Enable random + smooth, keep Speed constant, and compare the time to traverse
   the hue range. All displays should follow the same hue progression. If using
   a debugger, log `animation_hue` at `AnimationTimerProc`: exactly one advance
   per callback, irrespective of the window count.
3. Disable smooth but keep random enabled. All windows should change together
   on a random selection callback and hold together otherwise.
4. Run `matrix.scr /p <parent-HWND>` with a valid host window (for example the
   Windows screensaver preview). Confirm child rendering and all three color
   modes. Close the host and check that the process exits normally.
5. Open settings, change Hue/Random/Smooth in its embedded preview, then use
   Show to open fullscreen windows alongside it. Verify a single shared timer
   and identical hue on all targets, including the child preview.
6. Inspect first/last-window creation/destruction: one thread timer while any
   render targets exist, no timer after the last one, no stale matrix in the
   list. Reopen the preview/fullscreen session to verify initialization from
   the configured hue. Inject `SetTimer` failure in a debugger and confirm the
   failed window leaves no matrix/user-data entry.

These visual/lifecycle checks require Windows and are not covered by Linux
logic test success. No Windows application build or visual test was performed
in the implementation environment.
