/* global Group, Path, view */

import { ortho, Scene } from './scene.js'
import { complete, completeAll, Task } from './task.js'

const ctx = new Group({ data: { scenes: new Map() } })
const tasks = []

tasks.push(class extends Task {
  constructor (ctx) {
    super(ctx)
    this.id = '_0'
    const path = ctx.addChild(new Path({ name: this.id }))
    this.set(this.id, new Scene(path)
      .cylinder([
        [1, 0],
        [0, 1],
        [-1, 0],
        [0, -1]
      ]))

    this.get(this.id).project()
  }
})

tasks.push(class extends Task {
  constructor (ctx) {
    super(ctx)
    this.id = '_1'
    const path = ctx.addChild(new Path({ name: this.id }))
    this.set(this.id, new Scene(path))

    this.get(this.id)
      .cylinder([
        [1, 0],
        [0, 1],
        [-1, 0]
      ])
      .addParent(this.get('_0'))
      .project()
  }

  complete () {
    return complete(this.get(this.id).fold(180, { index: 2 }))
  }
})

tasks.push(class extends Task {
  constructor (ctx) {
    super(ctx)
    this.set('_2', new Scene(ctx.addChild(new Path({ name: '_2' }))))
      .set('_3', new Scene(ctx.addChild(new Path({ name: '_3' }))))

    this.get('_2')
      .cylinder([
        [1, 0],
        [0, -1],
        [0, 0]
      ])
      .addParent(this.get('_0'))
      .project()

    this.get('_3')
      .cylinder([
        [1, 0],
        [0, 1],
        [0, 0]
      ])
      .addParent(this.get('_1'))
      .project()
  }

  complete () {
    return Promise.all([
      this.get('_2').fold(180, { index: 1 }),
      this.get('_3').fold(180, { index: 1 })
    ].map(complete))
  }
})

tasks.push(class extends Task {
  constructor (ctx) {
    super(ctx)
    this.set('_4', new Scene(ctx.addChild(new Path({ name: '_4' }))))
      .set('_5', new Scene(ctx.addChild(new Path({ name: '_5' }))))

    this.get('_4')
      .cylinder([
        [1, 0],
        [0.5, -0.5],
        [0, 0]
      ])
      .addParent(this.get('_2'))
      .project()

    this.get('_5')
      .cylinder([
        [1, 0],
        [0.5, 0.5],
        [0, 0]
      ])
      .addParent(this.get('_3'))
      .project()
  }

  complete () {
    return Promise.all([
      this.get('_2').fold(90, { index: 1 }),
      this.get('_4').fold(90, { index: 1 })
    ].map(complete))
  }
})

ortho()
Scene.config({ scale: 100, center: [view.center.x, view.center.y] })
completeAll(ctx, tasks)
