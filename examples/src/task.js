/* global Group */

/**
 * Lifts a tween to promise.
 * @param {Tween} tween - An animation that repeatedly updates the state for a given duration.
 * @return {Promise} Resolves when the tween completes its animation.
 */
export const complete = tween => new Promise(resolve => tween.then(resolve))

export class Task extends Map {
  constructor (ctx) {
    if (!(ctx instanceof Group && ctx.data.scenes instanceof Map)) {
      throw new Error('Malformed context')
    }

    super(ctx.data.scenes)
  }

  complete () {
    return Promise.resolve()
  }
}

export const completeAll = async (ctx, tasks) => {
  for (const TaskImpl of tasks) {
    const task = new TaskImpl(ctx)
    await task.complete()

    for (const [id, scene] of task) {
      if (ctx.data.scenes.has(id)) continue
      ctx.data.scenes.set(id, scene)
    }
  }
}
