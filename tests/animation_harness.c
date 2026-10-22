#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#define VOID void
#define CALLBACK
#define _In_
typedef int HWND;
typedef unsigned int UINT;
typedef unsigned long UINT_PTR;
typedef unsigned long DWORD;
typedef long LONG;
#endif
#define HUE_MIN 1
#define HUE_MAX 255

typedef struct matrix {
    HWND hwnd;
    struct matrix *next;
    int rendered_hue;
    int frames;
} MATRIX, *PMATRIX;
static struct { int hue, is_random, is_smooth; } config;
static PMATRIX matrices;
static UINT_PTR animation_timer;
static LONG animation_hue;
static unsigned long fake_tick;
static int random_calls;
static unsigned long _r_sys_gettickcount(void) { return fake_tick; }
static int _r_math_getrandomrange(int min, int max) {
    ++random_calls;
    return min + (random_calls * 37) % (max - min + 1);
}
static void DecodeMatrix(HWND hwnd, PMATRIX matrix, LONG hue) {
    assert(hwnd == matrix->hwnd);
    matrix->rendered_hue = (int)hue;
    ++matrix->frames;
}
/* PRODUCTION_FUNCTIONS */

static void setup(MATRIX *windows, int count) {
    int i;
    config.hue = 85;
    config.is_random = 1;
    config.is_smooth = 1;
    animation_hue = config.hue;
    animation_timer = 42;
    matrices = windows;
    fake_tick = 1;
    random_calls = 0;
    for (i = 0; i < count; ++i) {
        windows[i].hwnd = (HWND)(UINT_PTR)(i + 1);
        windows[i].next = i + 1 < count ? &windows[i + 1] : NULL;
        windows[i].frames = 0;
        windows[i].rendered_hue = 85;
    }
}
static void check_frame(MATRIX *windows, int count, int frame, int hue) {
    int i;
    for (i = 0; i < count; ++i) {
        assert(windows[i].frames == frame);
        assert(windows[i].rendered_hue == hue);
    }
}
#ifndef _WIN32
int main(void) {
    MATRIX windows[8];
    int count, frame, expected;
    for (count = 1; count <= 8; ++count) {
        setup(windows, count);
        expected = 85;
        for (frame = 1; frame <= 1024; ++frame) {
            expected = expected >= HUE_MAX ? HUE_MIN : expected + 1;
            AnimationTimerProc(0, 0, 42, 0);
            check_frame(windows, count, frame, expected);
            assert(random_calls == 0);
        }
        /* Fixed color and live settings changes, including both endpoints. */
        config.is_random = 0;
        config.hue = HUE_MIN;
        AnimationTimerProc(0, 0, 42, 0);
        check_frame(windows, count, 1025, HUE_MIN);
        config.hue = HUE_MAX;
        AnimationTimerProc(0, 0, 42, 0);
        check_frame(windows, count, 1026, HUE_MAX);
        config.is_random = 1;
        AnimationTimerProc(0, 0, 42, 0);
        check_frame(windows, count, 1027, HUE_MIN);
        /* Preserve random jump policy: odd ticks choose once, even ticks hold. */
        config.is_smooth = 0;
        for (frame = 1028; frame <= 1127; ++frame) {
            fake_tick = (unsigned long)frame;
            expected = (int)animation_hue;
            AnimationTimerProc(0, 0, 42, 0);
            if (!(fake_tick % 2)) assert(animation_hue == expected);
            assert(animation_hue >= HUE_MIN && animation_hue <= HUE_MAX);
            check_frame(windows, count, frame, (int)animation_hue);
        }
        assert(random_calls == 50);
        /* Window-associated/stale callbacks cannot advance the shared clock. */
        AnimationTimerProc(0, 0, 99, 0);
        check_frame(windows, count, 1127, (int)animation_hue);
        animation_timer = 0;
        AnimationTimerProc(0, 0, 42, 0);
        check_frame(windows, count, 1127, (int)animation_hue);
        /* Removing/reordering windows does not transfer clock ownership. */
        animation_timer = 42;
        config.is_smooth = 1;
        matrices = &windows[count - 1];
        matrices->next = NULL;
        expected = animation_hue >= HUE_MAX ? HUE_MIN : (int)animation_hue + 1;
        AnimationTimerProc(0, 0, 42, 0);
        assert(matrices->frames == 1128 && matrices->rendered_hue == expected);
    }
    puts("shared animation: 1..8 windows, smooth/fixed/random/wrap/stale/removal passed");
    return 0;
}
#else
/* Real Windows message-loop integration, separate from portable logic tests. */
int main(void) {
    MATRIX windows[8];
    int count;
    for (count = 1; count <= 8; ++count) {
        MSG msg;
        DWORD started;
        setup(windows, count);
        animation_timer = SetTimer(NULL, 0, 10, AnimationTimerProc);
        assert(animation_timer);
        started = GetTickCount();
        while (windows[0].frames < 10 && GetTickCount() - started < 5000) {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
            else Sleep(1);
        }
        assert(KillTimer(NULL, animation_timer));
        animation_timer = 0;
        check_frame(windows, count, 10, 95);
    }
    puts("Windows SetTimer/DispatchMessage: 1..8 render targets passed");
    return 0;
}
#endif
