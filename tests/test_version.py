import pathlib
import re


def get_app_version():
    content = pathlib.Path('src/app.h').read_text(encoding='utf-8')
    match = re.search(r"#define\s+APP_VERSION\s+L\"([^\"]+)\"", content)
    assert match, 'APP_VERSION not found'
    return match.group(1)


def get_build_version():
    content = pathlib.Path('build.bat').read_text(encoding='utf-8')
    match = re.search(r"build\s+matrix\s+([\d\.]+)", content)
    assert match, 'Build script version not found'
    return match.group(1)


def test_versions_match():
    assert get_app_version() == get_build_version()
