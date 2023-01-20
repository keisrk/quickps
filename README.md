<p align="center">
  <img src="https://imgur.com/MjyBMrT.png" height="100" alt="quickps">
</p>

---

The swiss army knife of vector graphics scripting that fits in your pocket.

quickps is a combination of the power of Paper.js and small footprint of QuickJS.

## Installation

TBA

## Usage

TBA

## Development

- Requires Meson, ninja and curl to compile the project.
- Depends on `libcairomm-1.0-dev` and `libgtest-dev`.

```bash
git clone git@github.com:keisrk/quickps.git
cd quickps
meson setup build
meson compile -C build
meson test -C build
```

Optionally, activate a Python virtual environment and run more tests in `examples`
directory.

```bash
pip install -e ./examples
meson setup --reconfigure build
meson test -C build
```

It creates a test report (`./build/examples.html`).
