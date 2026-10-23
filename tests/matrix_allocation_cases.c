static void reset(size_t failure)
{
    assert(live == 0);
    attempts = freed = timer_calls = dc_calls = 0;
    fail_at = failure;
}
static void check_failure(size_t column)
{
    PMATRIX matrix;
    reset(column + 2); /* Outer matrix allocation precedes the columns. */
    matrix = CreateMatrix(NULL, GLYPH_WIDTH * 4, GLYPH_HEIGHT * 3);
    assert(matrix == NULL);
    assert(attempts == column + 2);
    assert(freed == column + 1 && live == 0);
    assert(dc_calls == 0 && timer_calls == 0);
}
int main(void)
{
    size_t i, y;
    PMATRIX matrix;
    check_failure(0);
    check_failure(2);
    check_failure(4);
    reset(1);
    assert(CreateMatrix(NULL, 56, 42) == NULL);
    assert(live == 0 && freed == 0);
    reset(0);
    matrix = CreateMatrix(NULL, 56, 42);
    assert(matrix && matrix->numcols == 5 && live == 6);
    for(i=0; i<matrix->numcols; ++i)
        for(y=0; y<matrix->numrows + 16; ++y)
            assert(matrix->column[i].glyph[y] == 0);
    DestroyMatrix(NULL, &matrix);
    assert(matrix == NULL && live == 0 && freed == 6);
#ifdef _WIN32
    {
        WNDCLASSW wc = {0};
        HWND hwnd;
        wc.lpfnWndProc = TestProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"MatrixAllocationRegression";
        assert(RegisterClassW(&wc));
        for(i=0; i<3; ++i)
        {
            reset(2 + i*2);
            hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP,
                                  0, 0, 56, 42, NULL, NULL, wc.hInstance, NULL);
            assert(hwnd == NULL && live == 0 && freed == fail_at - 1);
            assert(dc_calls == 0 && timer_calls == 0);
        }
        reset(0);
        hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP,
                              0, 0, 56, 42, NULL, NULL, wc.hInstance, NULL);
        assert(hwnd && live == 6 && timer_calls == 1);
        assert(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0);
        assert(DestroyWindow(hwnd));
        assert(live == 0 && freed == 6);
        assert(UnregisterClassW(wc.lpszClassName, wc.hInstance));
    }
#else
    for(i=0; i<3; ++i)
    {
        CREATESTRUCT cs = {56, 42};
        reset(2 + i*2);
        userdata = 0;
        assert(TestProc(NULL, WM_NCCREATE, 0, (LPARAM)&cs) == FALSE);
        assert(userdata == 0 && timer_calls == 0 && live == 0);
        assert(freed == fail_at - 1);
        TestProc(NULL, WM_NCDESTROY, 0, 0);
        assert(live == 0 && freed == fail_at - 1);
    }
#endif
    puts("matrix allocation regression checks passed");
    return 0;
}
