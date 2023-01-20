import subprocess


def test_qpsc(qpsc, js):
    png = js.parent / (js.stem + ".png")
    subprocess.run([qpsc, "--input", js, "--output", png]).check_returncode()
    assert png.exists()
