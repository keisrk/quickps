/* global Point */

import { m4, v3 } from 'twgl.js'

const viewProjection = m4.create()
const world = m4.identity()
const worldViewProjection = m4.create()
const origin = new Point()

export const rotate = () => {
  m4.multiply(
    m4.rotationX(35 * (Math.PI / 180)),
    m4.rotationY(-45 * (Math.PI / 180)),
    world)
}

export const perspective = () => {
  const fov = 30 * (Math.PI / 180)
  const aspect = 1.0
  const zNear = 0.5
  const zFar = 100.0
  const projection = m4.perspective(fov, aspect, zNear, zFar)
  const projection2d = m4.create()
  projection2d[0] = zNear
  projection2d[5] = zNear
  projection2d[10] = 1
  projection2d[14] = -1
  m4.multiply(projection, projection2d, viewProjection)
  m4.multiply(viewProjection, world, worldViewProjection)
  origin.set(project([0, 0, 0]))
}

export const ortho = () => {
  const zNear = -1
  const zFar = 1
  m4.ortho(-1, 1, -1, 1, zNear, zFar, viewProjection)
  m4.multiply(viewProjection, world, worldViewProjection)
  origin.set(project([0, 0, 0]))
}

export const project = v => {
  const [x, y] = m4.transformPoint(worldViewProjection, v)
  return new Point(x, -y)
}

const fromPoint = v => {
  if (v instanceof Float32Array) {
    return v
  } else {
    return v3.create(v[0], v[1], v.length === 3 ? v[2] : 0)
  }
}

const fromArray = arr => arr.map(fromPoint)

const edges = model => {
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

const edgeRotation = ([a, b], angle) => {
  const axis = v3.create()
  v3.subtract(b, a, axis)
  v3.normalize(axis, axis)
  const rad = angle * Math.PI / 180
  const m = m4.identity()
  m4.translate(m, v3.negate(a))
  m4.axisRotate(m, axis, rad, m)
  m4.translate(m, a)
  return m
}

const isFrontFace = ([a, b, c]) => a + b + c
const defaultFrontStyle = { fillColor: 'hsl(120,80%,25%)' }
const defaultBackStyle = { fillColor: 'hsl(240,100%,25%)' }
const config = {
  scale: 1.0,
  center: new Point(),
  frontStyle: defaultFrontStyle,
  backStyle: defaultBackStyle
}

export class Scene {
  static config ({ scale, center, frontStyle, backStyle }) {
    if (scale !== undefined) {
      config.scale = scale
    }

    if (center !== undefined) {
      config.center.set(center)
    }

    if (frontStyle !== undefined) {
      config.frontStyle = frontStyle
    }

    if (backStyle !== undefined) {
      config.backStyle = backStyle
    }
  }

  /**
   * @param {Path} path - The underlying path object.
   * @param {m4.Matrix} view - An optional view matrix.
   * @param {m4.Matrix} transform - An optional transform matrix.
   * @param {boolean} isFrontFace - An optional flag specifying whether the face is front or back.
   */
  constructor (path, { view = m4.identity(), transform = m4.identity(), isFrontFace = true } = {}) {
    this.path = path
    this.data = { view, transform, isFrontFace }
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
    const { model, view } = this.data
    this.path.segments = model.map(v => m4.transformPoint(view, v)).map(project)
    if (config.scale !== 1.0) this.path.scale(config.scale, origin)
    if (!config.center.isZero()) this.path.translate(config.center)
    return this
  }

  /**
   * Attaches a data attribute to the underlying Path instance.
   *
   * @param {Array} model - an array of v3.Vec3 or tuples.
   * @return {Scene} this instance for further method chaining.
   */
  cylinder (model) {
    this.data.model = fromArray(model)
    return this
  }

  transform (...args) {
    const { transform, view } = this.data

    for (const m of args) {
      m4.multiply(m, transform, transform)
    }

    if (this.data.parent !== undefined) {
      m4.multiply(this.data.parent.data.transform, transform, view)
    } else {
      m4.copy(transform, view)
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

    if (result.length === 0) {
      throw new Error('Unable to separate: submodel has no intersection.')
    }

    for (const [p, q] of result) {
      replace(this.data.model, p, q)
    }

    return this
  }

  addParent (scene) {
    this.data.parent = scene
    return scene.separate(this.data.model)
  }

  cull () {
    // https://www.cubic.org/docs/backcull.htm
    // https://www.khronos.org/opengl/wiki/Face_Culling
    if (isFrontFace(this.path.segments)) {
      this.path.set(config.frontStyle)
    } else {
      this.path.set(config.backStyle)
    }

    return this
  }

  onUpdate (delta, event) {
    if (this._tween_counter-- < 0) return this
    return this.transform(delta).project().cull()
  }

  fold (angle, { index = 0 }) {
    this._tween_counter = 30
    const { model } = this.data
    const [, ...edge] = subPath(model, model[index])
    const delta = edgeRotation(edge, angle / this._tween_counter)
    return this.path.tween(1000)
      .on('update', this.onUpdate.bind(this, delta))
  }
}
