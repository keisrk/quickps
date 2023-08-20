static char *CODE = R"(
  const results = [0, 1, 2, 3]
  let count = results.length

  const testCallsInOrder = () => {
    if (count !== results.pop()) throw new Error(`${count}`)
  }

  // eslint-disable-next-line no-unused-vars
  for (const i of Array(results.length).keys()) {
    globalThis.queueMicrotask(() => count--)
    globalThis.queueMicrotask(testCallsInOrder)
  }
)";
