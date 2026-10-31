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

TBA

## Usage

TBA

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

### Configuration Options

You can pass options during `meson setup` or update an existing build directory with `meson configure build -D<option>=<value>`.

| Option | Allowed Values | Description |
| :--- | :--- | :--- |
| `b_sanitize` | `none` (default), `address`, `undefined`, `address,undefined` | Enables compiler sanitizers. When set to `address`, memory errors (out-of-bounds, use-after-free) are tracked during test runs. Note that `qpsc` is dynamically linked under sanitizers. When `none`, `qpsc` is statically linked. |
| `buildtype` | `debug` (default), `release`, `debugoptimized`, `plain` | Sets optimization and debug symbol levels. Use `release` for optimized builds (`-O3`). |

#### Testing with Sanitizers

When AddressSanitizer is enabled, `unit_test` and `integration test` will detect memory safety issues:

```bash
# Enable AddressSanitizer
meson configure build -Db_sanitize=address
meson compile -C build

# Run tests (default setup: checks memory errors without leak noise)
meson test -C build

# Run tests with leak detection enabled
meson test -C build --setup=leakcheck
```

#### Standalone Portable Release Build

To build a standalone, statically linked `qpsc` binary for release:

```bash
meson configure build -Db_sanitize=none --buildtype=release
meson compile -C build
```
