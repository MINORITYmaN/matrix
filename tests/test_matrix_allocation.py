import os
import shlex
import shutil
import subprocess

import pytest

from matrix_allocation_harness import harness


def test_matrix_allocation_failure(tmp_path):
    if os.name == 'nt':
        pytest.skip('Native Windows lifecycle: python tests/windows/run_allocation.py')
    compiler = shlex.split(os.environ.get('CC', 'cc'))
    if not shutil.which(compiler[0]):
        pytest.skip('A C compiler is required')
    source = tmp_path / 'allocation.c'
    executable = tmp_path / 'allocation'
    source.write_text(harness())
    subprocess.run([*compiler, '-std=c99', '-Wall', '-Wextra', '-Werror',
                    str(source), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
