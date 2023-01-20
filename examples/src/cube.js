/* global CompoundPath, Path, view */
import { project } from './scene.js'
import { m4, v3 } from 'twgl.js'

class Cube extends CompoundPath {
  constructor (r) {
    super()

    const points = []

    for (const i of Array(8).keys()) {
      const sign = i % 8 < 4 ? 1 : -1
      const angle = (i % 4) * 90 * sign
      const v = v3.create(sign * r, sign * r, sign * r)
      const m = m4.identity()
      m4.rotateY(m, angle * (Math.PI / 180), m)
      m4.transformPoint(m, v, v)
      points.push(v)
    }

    for (const i of Array(3).keys()) {
      const color = `hsl(240,50%,${30 + i * 10}%)`
      this.children.push(new Path({ fillColor: color }))
    }

    this.children[0].segments = points.filter(p => p[0] === r).map(project)
    this.children[1].segments = points.filter(p => p[1] === r).map(project)
    this.children[2].segments = points.filter(p => p[2] === -r).map(project)

    this.bounds.center = view.center
  }
}

/* eslint-disable no-new */
new Cube(50)
/* eslint-enable no-new */
