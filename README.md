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
