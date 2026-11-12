import { Document, HTMLElement } from 'nodom.js';
import { Canvas, CanvasRenderingContext2D, Image, createCanvas } from 'canvas';

export function defineScope(config) {
  config = config || {};
  let width = config.width || 640;
  let height = config.height || 480;
  if (config.size && Array.isArray(config.size)) {
    width = config.size[0] || width;
    height = config.size[1] || height;
  }
  const cvs = createCanvas(width, height, 'image');
  if (typeof config.onDraw === 'function') {
    cvs._onDraw = config.onDraw;
  } else if (config.output) {
    cvs._onDraw = function (c) {
      c.write(config.output);
    };
  }

  // Pre-load and register images KV (e.g. { mona: './assets/mona.png', logo: '...' })
  if (config.images && typeof config.images === 'object') {
    const documentObj = globalThis.document || doc;
    if (Array.isArray(config.images)) {
      for (const item of config.images) {
        if (typeof item === 'string') {
          const id = item.replace(/^.*[\\/]/, '').replace(/\.[^/.]+$/, '');
          documentObj.registerImage(id, item);
        } else if (item && item.id && item.src) {
          documentObj.registerImage(item.id, item.src);
        }
      }
    } else {
      for (const [id, src] of Object.entries(config.images)) {
        documentObj.registerImage(id, src);
      }
    }
  }

  return cvs;
}

const origCreateElement = Document.prototype.createElement;
Document.prototype.createElement = function (tagName) {
  if (tagName === 'canvas') {
    return createCanvas(300, 150, 'image');
  }
  if (tagName === 'img' || tagName === 'image') {
    return new Image();
  }
  return origCreateElement.call(this, tagName);
};

const origGetElementById = Document.prototype.getElementById;
Document.prototype.getElementById = function (id) {
  if (this._images && this._images[id]) {
    return this._images[id];
  }
  return origGetElementById.call(this, id);
};

Document.prototype.registerImage = function (id, imageOrSrc) {
  if (!this._images) {
    this._images = {};
  }
  let img;
  if (typeof imageOrSrc === 'string') {
    img = new Image();
    img.id = id;
    if (typeof img.load === 'function') {
      img.load(imageOrSrc);
    } else {
      img.src = imageOrSrc;
    }
  } else {
    img = imageOrSrc;
    img.id = id;
  }
  this._images[id] = img;
  return img;
};

HTMLElement.prototype.setAttributeNS = function (ns, name, value) {
  return this.setAttribute(name, value);
};

HTMLElement.prototype.getAttributeNS = function (ns, name) {
  return this.getAttribute(name);
};

Document.prototype.addEventListener = function () {};

Document.prototype.defaultView = {
  getComputedStyle: function () {
    return {};
  }
};

globalThis.setInterval = function () {};
globalThis.clearInterval = function () {};
globalThis.setTimeout = function () {};
globalThis.clearTimeout = function () {};

const doc = new Document();
globalThis.document = doc;
globalThis.Image = Image;

globalThis.self = {
  acorn: undefined,
  Canvas: Canvas,
  HTMLCanvasElement: Canvas,
  CanvasRenderingContext2D: CanvasRenderingContext2D,
  Image: Image,
  document: doc,
  navigator: {
    userAgent: 'QuickJS'
  },
  window: {
    navigator: {
      userAgent: 'QuickJS'
    },
    Canvas: Canvas,
    HTMLCanvasElement: Canvas,
    CanvasRenderingContext2D: CanvasRenderingContext2D,
    Image: Image,
    document: doc,
    addEventListener: function () {},
    requestAnimationFrame: function () {},
    cancelAnimationFrame: function () {},
    setInterval: function () {},
    clearInterval: function () {},
    setTimeout: function () {},
    clearTimeout: function () {}
  },
  XMLSerializer: function () {
    this.serializeToString = function (node) {
      return node.outerHTML;
    };
  },
};
globalThis.window = globalThis.self.window;
