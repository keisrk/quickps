import pytest
from pathlib import Path


def pytest_addoption(parser):
    parser.addoption("--qpsc", type=Path, required=True, help="path to qpsc executable")
    parser.addoption("--js", type=Path, action="append", help="path to input JS file")
    parser.addoption(
        "--image",
        action="append",
        default=[],
        help="pre-load image in id=path or path format",
    )


def pytest_generate_tests(metafunc):
    if "js" in metafunc.fixturenames:
        js_files = metafunc.config.getoption("js")
        metafunc.parametrize("js", js_files)


@pytest.fixture
def qpsc(request):
    return request.config.getoption("qpsc")


@pytest.fixture
def images(request):
    return request.config.getoption("image")
