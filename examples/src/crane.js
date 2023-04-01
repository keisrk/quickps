/* global Group, Path, view */

import { Scene } from './scene.js'
import { m4 } from 'twgl.js'

const ctx = new Group()
const tasks = []

const isFrontSide = (p, a, b) => p.rotate((b - a).angle * -1, a).y < a.y

tasks.push(class {
  constructor (ctx) {
    this.name = '_ix'
    const path = ctx.addChild(new Path({ name: this.name, fillColor: 'hsl(120,80%,25%)' }))
    this.scene = new Scene(path)
      .cylinder([
        [1, 0],
        [0, 1],
        [-1, 0],
        [0, -1]
      ])
  }

  next () {
    this.scene
      .project()
    return { done: true }
  }
})

tasks.push(class {
  constructor (ctx, history) {
    this.name = '_u6'
    const path = ctx.addChild(new Path({ name: this.name }))
    this.scene = new Scene(path)
      .cylinder([
        [1, 0],
        [0, 1],
        [-1, 0]
      ])

    history.get('_ix').scene
      .separate(this.scene.data.model)
      .project()
  }

  next () {
    const angle = 180
    this.scene
      .transform(
        m4.rotationX(angle * (Math.PI / 180))
      )
      .project()
    const [{ point: a }, { point: p }, { point: b }] = this.scene.path.segments
    this.scene.path.fillColor = isFrontSide(p, a, b) ? 'hsl(240,100%,50%)' : 'hsl(120,100%,25%)'
    return { done: true }
  }
})

tasks.push(class {
  constructor (ctx, history) {
    this.name = '_nl'
    this.scenes = new Map()
      .set('_gy', new Scene(ctx.addChild(new Path({ name: '_gy', fillColor: 'hsl(240,100%,50%)' })))
        .cylinder([
          [1, 0],
          [0, -1],
          [0, 0]
        ],
        history.get('_ix').scene.data.transform
        ))
      .set('_rs', new Scene(ctx.addChild(new Path({ name: '_rs', fillColor: 'hsl(240,100%,25%)' })))
        .cylinder([
          [1, 0],
          [0, 1],
          [0, 0]
        ],
        history.get('_u6').scene.data.transform
        ))

    history.get('_ix').scene
      .separate(this.scenes.get('_gy').data.model)
      .project()

    history.get('_u6').scene
      .separate(this.scenes.get('_rs').data.model)
      .project()
  }

  next () {
    const angle = 180

    for (const scene of this.scenes.values()) {
      scene
        .transform(m4.rotationY(angle * (Math.PI / 180)))
        .project()
    }

    return { done: true }
  }
})

const history = new Map()

for (const Task of tasks) {
  const task = new Task(ctx, history)

  /* eslint-disable no-unused-vars, no-empty */
  for (let { done } = task.next(); !done; { done } = task.next()) {}
  /* eslint-enable no-unused-vars, no-empty */

  if (typeof task.terminate === 'function') task.terminate(ctx)

  history.set(task.name, task)
}

ctx.translate([view.center.x, -view.center.y])
ctx.scale(100, view.center)
