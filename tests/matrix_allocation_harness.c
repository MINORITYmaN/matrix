#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#define _In_
#define _Inout_
#define CALLBACK
#define TRUE 1
#define FALSE 0
#define VOID void
#define WM_NCCREATE 1
#define WM_NCDESTROY 2
#define GWLP_USERDATA 0
#define BLACKNESS 0
#define RtlZeroMemory(p,n) memset(p,0,n)
typedef uint32_t ULONG, *PULONG;
typedef int32_t LONG;
typedef uintptr_t ULONG_PTR;
typedef intptr_t LONG_PTR, LPARAM, LRESULT;
typedef uintptr_t WPARAM;
typedef unsigned int UINT;
typedef int INT, BOOLEAN;
typedef void *HWND, *HDC, *HBITMAP;
typedef struct { int cx, cy; } CREATESTRUCT, *LPCREATESTRUCT;
static LONG_PTR userdata;
static HDC GetDC(HWND w) { (void)w; return (HDC)1; }
static int ReleaseDC(HWND w, HDC dc) { (void)w; (void)dc; return 1; }
static HDC CreateCompatibleDC(HDC dc) { (void)dc; return (HDC)2; }
static HBITMAP CreateCompatibleBitmap(HDC dc, ULONG w, ULONG h) { (void)dc; (void)w; (void)h; return (HBITMAP)3; }
static void *SelectObject(HDC dc, HBITMAP b) { (void)dc; (void)b; return NULL; }
static int PatBlt(HDC dc, int x, int y, ULONG w, ULONG h, int op) { (void)dc; (void)x; (void)y; (void)w; (void)h; (void)op; return 1; }
static int DeleteDC(HDC dc) { (void)dc; return 1; }
static int DeleteObject(HBITMAP b) { (void)b; return 1; }
static LONG_PTR SetWindowLongPtrW(HWND w, int i, LONG_PTR p) { (void)w; (void)i; userdata=p; return 0; }
static LONG_PTR GetWindowLongPtr(HWND w, int i) { (void)w; (void)i; return userdata; }
static int KillTimer(HWND w, unsigned int id) { (void)w; (void)id; return 1; }
static HWND GetParent(HWND w) { (void)w; return NULL; }
static void PostQuitMessage(int code) { (void)code; }
static LRESULT DefWindowProcW(HWND w, UINT m, WPARAM a, LPARAM b) { (void)w; (void)m; (void)a; (void)b; return 0; }
#endif
#define RND_MAX INT_MAX
#define IDR_GLYPH 1
static size_t attempts, fail_at, live, freed, timer_calls, dc_calls;
static void *blocks[64];
static void *_r_mem_allocate(size_t size)
{
    void *p;
    if (++attempts == fail_at) return NULL;
    p = malloc(size);
    assert(p && live < 64);
    memset(p, 0xa5, size); /* Require production code to initialize pointers. */
    blocks[live++] = p;
    return p;
}
static void _r_mem_free(void *p)
{
    size_t i;
    for (i=0; i<live && blocks[i]!=p; ++i) {}
    assert(i<live); /* Reject invalid pointers and double frees. */
    blocks[i] = blocks[--live];
    ++freed;
    free(p);
}
static LONG _r_math_getrandomrange(LONG a, LONG b) { (void)a; (void)b; return 0; }
static void *_r_sys_getimagebase(void) { return NULL; }
static HBITMAP MakeBitmap(HDC dc, void *base, int id, int hue)
{ (void)base; (void)id; (void)hue; return CreateCompatibleBitmap(dc, 14, 14); }
static HDC tracked_GetDC(HWND hwnd) { ++dc_calls; return GetDC(hwnd); }
#define GetDC tracked_GetDC
#ifdef _WIN32
static UINT_PTR tracked_SetTimer(HWND w, UINT_PTR id, UINT delay, TIMERPROC proc)
{ ++timer_calls; return SetTimer(w, id, delay, proc); }
#else
static uintptr_t tracked_SetTimer(HWND w, unsigned int id, UINT delay, int proc)
{ (void)w; (void)id; (void)delay; (void)proc; ++timer_calls; return 1; }
#endif
#define SetTimer tracked_SetTimer
