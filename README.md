<p align="center">
  <img src="https://imgur.com/MjyBMrT.png" height="100" alt="quickps">
</p>

---

The swiss army knife of vector graphics scripting that fits in your pocket.

quickps combines the power of [Paper.js](http://paperjs.org/) and small footprint of [QuickJS](https://bellard.org/quickjs/).

It is an image processing program that takes PaperScript as input, processes it using the Paper.js library in conjunction
with QuickJS runtime and generates a canvas image as output.

## What is PaperScript?

[PaperScript](http://paperjs.org/tutorials/getting-started/working-with-paper-js/) is the plain old JavaScript with some
built-in niceties;

<dl>
<dt>Batteries included</dt>
<dd>

`Path`, `Shape`, `Text`, etc. are basic building blocks in Paper.js and are always available in the scope of your script.
Just instantiate one them and Paper.js draws it on the canvas right away.

</dd>

<dt>Operator overloading</dt>
<dd>

`Point` and `Size` play a central role in Paper.js when exploring the 2D canvas space. They accept the basic arithmetic
operators (`+`, `-`, `*`, `/` and `%`). This makes it easy and intuitive to script your graphical ideas. A vector is a first
class citizen in PaperScript.

</dd>

<dt>Tweening</dt>
<dd>

In PaperScript, you can use `Tween` to animate any property of a Paper.js object, such as its position, size, color, or
opacity. Simply specify the starting and ending states of the object, as well as the duration of the transition. The `Tween`
will then automatically update the property over the course of the specified duration.

</dd>
</dl>

## Installation

Download the precompiled static binary from [GitHub Releases](https://github.com/keisrk/quickps/releases):

```bash
# Extract and install binary to ~/.local/bin
tar -xzf quickps-*-linux-x86_64.tar.gz
mkdir -p ~/.local/bin
mv quickps-*-linux-x86_64/qpsc ~/.local/bin/
```

Or build from source (see [Development](#development)).

## Usage

Compile a PaperScript file into a raster (PNG) or vector (SVG) image:

```bash
# Render to PNG (default size: 640x480)
qpsc -i script.js -o output.png

# Export to SVG
qpsc -i script.js -o output.svg

# Specify canvas dimensions
qpsc -i script.js -o output.png -s 1280x720

# Pipe from stdin
cat script.js | qpsc -o output.png
```

## Development

- Requires Meson, ninja and curl to compile the project.

```bash
git clone git@github.com:keisrk/quickps.git
cd quickps
CC=clang-20 CXX=clang++-20 meson setup build
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

### Build Profiles

| Profile | Configuration Flags | Description |
| :--- | :--- | :--- |
| **Debug with sanitizers** | `-Db_sanitize=address` | Development & testing. Dynamically linked with AddressSanitizer enabled. |
| **Release** | `--buildtype=release` | Distribution. Optimized (`-O3`) with statically linked `qpsc`. |

```bash
# Debug with sanitizers (tests catch memory errors)
meson configure build -Db_sanitize=address
meson test -C build

# Release (statically linked binary)
meson configure build --buildtype=release
meson compile -C build
```
