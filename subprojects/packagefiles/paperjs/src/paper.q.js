globalThis.self = {
  navigator: {
    userAgent: 'QuickJS'
  }
}

globalThis.global = globalThis.self;

/*#*/ include('../node_modules/nodom/nodom.js')

globalThis.self.nodom.HTMLElement.prototype.setAttributeNS = function() {}
globalThis.self.nodom.Document.prototype.addEventListener = function() {}
globalThis.self.document = new globalThis.self.nodom.Document();

(function() {

/*#*/ include('options.js')
/*#*/ include('paper.js')
this.paper = paper

}).call(globalThis);

globalThis.paper.DomEvent.requestAnimationFrame = function(callback) {}
globalThis.paper.PaperScope.inject({
  createCanvas: (width, height, type) => {
    width,
    height,
    type
  }
})

export const paper = globalThis.paper
