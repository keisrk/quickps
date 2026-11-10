import subprocess
import pytest


@pytest.mark.parametrize("ext", [".png", ".svg"])
def test_qpsc(qpsc, js, ext, images):
    out = js.parent / (js.stem + ext)
    cmd = [str(qpsc), "--input", str(js), "--output", str(out)]
    for img in images:
        cmd.extend(["--image", str(img)])
    subprocess.run(cmd).check_returncode()
    assert out.exists()
    if ext == ".svg":
        content = out.read_text()
        assert "<svg" in content
        assert 'xmlns="http://www.w3.org/2000/svg"' in content
        assert "</svg>" in content


def test_qpsc_img_prefix_syntax(qpsc, js, tmp_path, images):
    if not images or js.stem != "smiley":
        return
    out = tmp_path / "test_prefix.png"
    cmd = [str(qpsc), "--input", str(js), "--output", str(out)]
    for img in images:
        cmd.append(f"--img:{img}")
    subprocess.run(cmd).check_returncode()
    assert out.exists()
