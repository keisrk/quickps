import subprocess
import pytest


@pytest.mark.parametrize("ext", [".png", ".svg"])
def test_qpsc(qpsc, js, ext):
    out = js.parent / (js.stem + ext)
    subprocess.run([qpsc, "--input", js, "--output", out]).check_returncode()
    assert out.exists()
    if ext == ".svg":
        content = out.read_text()
        assert "<svg" in content
        assert 'xmlns="http://www.w3.org/2000/svg"' in content
        assert "</svg>" in content
