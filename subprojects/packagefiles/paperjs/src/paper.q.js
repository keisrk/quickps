/* global include */

globalThis.self = {
  navigator: {
    userAgent: 'QuickJS'
  },
  window: {
    navigator: {
      userAgent: 'QuickJS'
    },
    addEventListener: function () {}
  }
}

/* eslint-disable */
globalThis.global = globalThis.self;
/*#*/ include('../node_modules/nodom/nodom.js')
/* eslint-enable */

globalThis.self.nodom.HTMLElement.prototype.setAttributeNS = function () {}
globalThis.self.nodom.HTMLElement.prototype.getContext = function (toodee) {
  return {
    save: function () {},
    restore: function () {}
  }
}

globalThis.self.nodom.Document.prototype.addEventListener = function () {}

/* eslint-disable */
globalThis.self.document = new globalThis.self.nodom.Document();
(function () {
  /*#*/ include('options.js')
  /*#*/ include('paper.js')
  this.paper = paper
}).call(globalThis)
/* eslint-enable */

globalThis.paper.DomEvent.requestAnimationFrame = function (callback) {}
globalThis.paper.PaperScope.inject({
  createCanvas: (width, height, type) => {
    return { width, height, type }
  }
})

export const paper = globalThis.paper
