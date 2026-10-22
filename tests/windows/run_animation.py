"""Run from a Visual Studio Developer Command Prompt: python tests/windows/run_animation.py."""
import pathlib
import subprocess
import sys
import tempfile

assert sys.platform == 'win32', 'This integration test requires Windows and MSVC'
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))
from test_animation import ROOT, animation_source

with tempfile.TemporaryDirectory() as directory:
    work = pathlib.Path(directory)
    source = work / 'animation.c'
    source.write_text((ROOT / 'tests/animation_harness.c').read_text().replace(
        '/* PRODUCTION_FUNCTIONS */', animation_source()))
    subprocess.run(['cl', '/nologo', '/W4', '/TC', str(source),
                    '/Fe:animation.exe', 'user32.lib'], cwd=work, check=True)
    subprocess.run([str(work / 'animation.exe')], check=True)
