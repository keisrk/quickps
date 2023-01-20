/* global CompoundPath, Group, Path, Point, view */

const r = 50
const rad1 = 22.5 * (Math.PI / 180)
const rad2 = 67.5 * (Math.PI / 180)
const inR = Math.sin(rad1) * r
const gap = (Math.cos(rad1) - Math.cos(rad2)) * r
const span = r * 0.15
const style = {
  fillColor: '#E82746',
  fillRule: 'evenodd'
}

class Q extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [0, r]
    let end
    this.children = [
      new Path([
        p = p.rotate(-22.5, c),
        end = p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p += [span, 0],
        p = c + inR,
        p += [0, gap],
        p += gap,
        end
      ]),
      new Path.Rectangle(
        c + inR,
        c - inR
      )
    ]
  }
}

class U extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [0, -r]
    const end = p.rotate(-22.5, c)
    this.children = [
      new Path([
        end - [gap, 0],
        p = end.rotate(-90, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p.rotate(-45, c)
      ]),
      new Path.Rectangle(
        end,
        c + inR
      )
    ]
  }
}

class I extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [r, 0]
    let end
    this.children = [
      new Path([
        end = p = p.rotate(-112.5, c),
        p - [gap, 0],
        p = p.rotate(-90, c),
        p += gap,
        end
      ]),
      new Path([
        p = end - [0, span],
        p -= gap,
        p += [gap, 0]
      ])
    ]
  }
}

class C extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [r, 0]
    const end = p.rotate(-22.5, c)
    this.children = [
      new Path([
        p = end,
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p.rotate(-45, c)
      ]),
      new Path.Rectangle(
        c + [-inR, inR],
        end
      )
    ]
  }
}

class K extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [r, 0]
    let end
    this.children = [
      new Path([
        end = p = p.rotate(-112.5, c),
        p -= gap,
        p = end.rotate(-90, c),
        p += gap,
        end
      ]),
      new Path([
        end = p = c - [inR, 0],
        p += [gap, -gap],
        p += [2 * inR, 0],
        p += [-gap, gap],
        p += [-span, span],
        p += gap + span,
        p -= [2 * inR, 0],
        p -= gap,
        end
      ])
    ]
  }
}

class P extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    let p = c + [0, r]
    const end = p.rotate(22.5, c)
    this.children = [
      new Path([
        p = end,
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p = p.rotate(-45, c),
        p += [0, gap],
        p += gap,
        end
      ]),
      new Path([
        p = end,
        p += [2 * inR - span, 0],
        c + [-inR, inR],
        end
      ]),
      new Path.Rectangle(
        c + inR,
        c - inR
      )
    ]
  }
}

class S extends CompoundPath {
  constructor (x, y) {
    super(style)
    const c = new Point(x, y)
    const margin = inR * 2 / 3
    const d = c + [0, margin]
    let p = c + [r, 0]
    const end = p.rotate(22.5, c)
    this.children = [
      new Path([
        p = end,
        p += [0, margin],
        p = p.rotate(45, d),
        p = p.rotate(45, d),
        p = p.rotate(45, d),
        p -= [0, margin],
        p = p.rotate(45, c),
        p = p.rotate(45, c),
        p = p.rotate(45, c),
        p = p.rotate(45, c),
        end
      ]),
      new Path.Rectangle(
        c + inR,
        c - inR
      ),
      new Path.Rectangle(
        d + inR,
        d - inR
      ),
      new Path([
        p = c + [inR, -inR],
        p += [gap, 0],
        p += [0, gap + margin],
        p -= gap
      ]),
      new Path([
        p = d + [-inR, inR],
        p -= [gap, 0],
        p -= [0, gap + margin],
        p += gap
      ])
    ]
  }
}

class Logo extends Group {
  constructor () {
    super()
    let glyph
    this.children = [
      glyph = new Q(0, 0),
      glyph = new U(glyph.bounds.topRight.x + r, 0),
      glyph = new I(glyph.bounds.topRight.x + r, 0),
      glyph = new C(glyph.bounds.topRight.x + r, 0),
      glyph = new K(glyph.bounds.topRight.x + r, 0),
      glyph = new P(glyph.bounds.topRight.x + r, 0),
      glyph = new S(glyph.bounds.topRight.x + r, 0)
    ]
    this.bounds.center = view.center
  }
}

/* eslint-disable no-new */
new Logo()
/* eslint-enable no-new */
