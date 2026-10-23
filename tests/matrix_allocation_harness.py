"""Compile the production lifecycle functions with a fault-injecting allocator."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def function(source, signature):
    start = source.index(signature)
    body = source.index('{', start)
    depth = 1
    end = body + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def harness():
    source = (ROOT / 'src/main.c').read_text()
    header = (ROOT / 'src/main.h').read_text()
    header = '\n'.join(line for line in header.splitlines()
                       if not line.startswith(('#include', '#pragma')))
    create = function(source, 'PMATRIX CreateMatrix (')
    # The forward declaration precedes CreateMatrix; select the definition.
    destroy = function(source[source.index(create) + len(create):],
                       'VOID DestroyMatrix (')
    nccreate = source[source.index('case WM_NCCREATE:'):source.index('case WM_NCDESTROY:')]
    ncdestroy = source[source.index('case WM_NCDESTROY:'):source.index('case WM_CLOSE:')]
    prelude = (ROOT / 'tests/matrix_allocation_harness.c').read_text()
    return (prelude + '\n' + header + '\nSTATIC_DATA config = {0};\n'
            'VOID DestroyMatrix(HWND hwnd, PMATRIX *matrix);\n' + create + '\n' + destroy
            + '\nLRESULT CALLBACK TestProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)\n'
            '{ PMATRIX matrix; BOOLEAN is_savecursor = FALSE; (void)wparam; (void)is_savecursor;\n'
            'switch(msg) {\n' + nccreate + ncdestroy
            + '\ndefault: return DefWindowProcW(hwnd, msg, wparam, lparam); }}\n'
            + (ROOT / 'tests/matrix_allocation_cases.c').read_text())
