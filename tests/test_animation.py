"""Compile the production hue/timer functions with a recording renderer.

No Win32 or graphics libraries are needed. The Windows integration suite uses
these same functions with real SetTimer/DispatchMessage instead.
"""
import os
import pathlib
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]


def animation_source():
    source = (ROOT / 'src/main.c').read_text()
    return source[source.index('VOID UpdateAnimationHue ('):source.index('LRESULT CALLBACK ScreensaverProc (')]


def test_shared_animation(tmp_path):
    compiler = shutil.which(os.environ.get('CC', 'cc'))
    assert compiler, 'A C compiler is required (set CC if necessary)'
    harness = (ROOT / 'tests/animation_harness.c').read_text()
    source = tmp_path / 'animation.c'
    source.write_text(harness.replace('/* PRODUCTION_FUNCTIONS */', animation_source()))
    executable = tmp_path / 'animation'
    subprocess.run([compiler, '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-Wno-unused-parameter', str(source), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)


def test_window_timer_wiring():
    """Guard the connection between the tested dispatcher and the window code."""
    source = (ROOT / 'src/main.c').read_text()
    assert source.count('SetTimer (') == 1
    assert 'SetTimer (NULL, 0,' in source
    assert '* 10, AnimationTimerProc)' in source
    assert 'case WM_TIMER:' not in source
    assert 'KillTimer (NULL, animation_timer)' in source
    decode = source[source.index('VOID DecodeMatrix ('):source.index('PMATRIX CreateMatrix (')]
    assert 'SetMatrixBitmap (matrix, hue)' in decode
    assert 'config.is_random' not in decode
    assert 'new_hue' not in decode
