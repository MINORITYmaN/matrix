"""Run from an MSVC Developer Command Prompt on Windows."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from matrix_allocation_harness import harness

if os.name != 'nt':
    raise SystemExit('This harness requires Windows and MSVC; use pytest on other systems.')

with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    source = work / 'allocation.c'
    source.write_text(harness())
    subprocess.run(['cl', '/nologo', '/W4', '/TC', str(source),
                    '/Fe:allocation.exe', '/link', 'user32.lib', 'gdi32.lib'],
                   cwd=work, check=True)
    subprocess.run([str(work / 'allocation.exe')], cwd=work, check=True)
