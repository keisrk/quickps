/* global Point, view */

import { m4, v3 } from 'twgl.js'

export const project = ([x, y, z]) => {
  const rad = 30 * (Math.PI / 180)
  const a = 0.5 * Math.cos(rad)
  const b = 0.5 * Math.sin(rad)
  const newX = x + a * z
  const newY = view.size.height - (y + b * z)
  return new Point(newX, newY)
}

const fromPoint = (v) => {
  if (v instanceof Float32Array) {
    return v
  } else {
    return v3.create(v[0], v[1], v.length === 3 ? v[2] : 0)
  }
}

const fromArray = (arr) => arr.map(fromPoint)

const edges = (model) => {
  const edges = []

  for (const i of Array(model.length - 1).keys()) {
    const slug = i + 1

    for (const j of Array(model.length - slug).keys()) {
      edges.push([model[i], model[slug + j]])
    }
  }

  return edges
}

const includes = (model, p) => {
  const i = model.findIndex(r => v3.distance(p, r) === 0)
  return i !== -1
}

/** Returns [a, p, b]. */
const subPath = (model, p) => {
  const iP = model.findIndex(r => v3.distance(p, r) === 0)
  if (iP === -1) {
    throw new Error(`Point ${p} not found in a model ${model}`)
  }

  const iA = iP === 0 ? model.length - 1 : iP - 1
  const iB = iP === model.length - 1 ? 0 : iP + 1
  return [model[iA], p, model[iB]]
}

const replace = (model, p, q) => {
  const i = model.findIndex(r => v3.distance(p, r) === 0)

  if (q === undefined) {
    return model.splice(i, 1)
  } else {
    return v3.copy(q, model[i])
  }
}

export class Scene {
  constructor (path) {
    this.path = path
  }

  get data () {
    return this.path.data
  }

  set data (data) {
    this.path.data = data
  }

  /**
   * Maps a 3D model to an array of 2D points.
   *
   * @return {Scene} this instance for further method chaining.
   */
  project () {
    const { model, transform } = this.data
    this.data.target = model.map(v => m4.transformPoint(transform, v))
    this.path.segments = this.data.target.map(project)
    return this
  }

  /**
   * Attaches a data attribute to the underlying Path instance.
   *
   * @param {Array} model - an array of v3.Vec3 or tuples.
   * @param {m4.Matrix} transform - An optional transform matrix.
   * @return {Scene} this instance for further method chaining.
   */
  cylinder (model, transform = m4.identity()) {
    this.data = { model: fromArray(model), transform }
    return this
  }

  transform (...args) {
    const { transform } = this.data

    for (const m of args) {
      m4.multiply(m, transform, transform)
    }

    return this
  }

  separate (subModel) {
    // Returns a model s.t. no interscetion with the other model and the union
    // covers the original model.
    const p2a = v3.create()
    const p2b = v3.create()
    const p2edge = v3.create()
    const result = []

    for (const p of subModel) {
      // Check if this model has p and shares paths from p with the other.
      if (includes(this.data.model, p)) {
        const [a1,, b1] = subPath(this.data.model, p)
        const [a2,, b2] = subPath(subModel, p)
        if (
          (v3.distance(a1, a2) === 0 && v3.distance(b1, b2) === 0) ||
          (v3.distance(a1, b2) === 0 && v3.distance(b1, a2) === 0)
        ) {
          result.push([p])
        }

        continue
      }

      // Check if an edge in this model contains p.
      for (const [a, b] of edges(this.data.model)) {
        v3.subtract(a, p, p2a)
        v3.subtract(b, p, p2b)
        v3.cross(p2a, p2b, p2edge)

        if (v3.length(p2edge) !== 0) {
          continue
        }

        if (includes(subModel, a)) {
          // Replace a with p
          result.push([a, p])
        } else if (includes(subModel, b)) {
          // Replace b with p
          result.push([b, p])
        } else {
          throw new Error(`Invalid point a: ${a}, b: ${b}, p: ${p}.`)
        }
      }
    }

    for (const [p, q] of result) {
      replace(this.data.model, p, q)
    }

    return this
  }
}
