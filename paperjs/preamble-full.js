import { Document, HTMLElement } from 'nodom.js';
import * as acorn from 'acorn.js';
import { Canvas, CanvasRenderingContext2D, Image, createCanvas } from 'canvas';

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

HTMLElement.prototype.setAttributeNS = function (
  ns,
  name,
  value
) {
  return this.setAttribute(name, value);
};

HTMLElement.prototype.getAttributeNS = function (
  ns,
  name
) {
  return this.getAttribute(name);
};

Document.prototype.addEventListener = function () {}

Document.prototype.defaultView = {
  getComputedStyle: function () {
    return {}
  }
}

globalThis.setInterval = function () {};
globalThis.clearInterval = function () {};
globalThis.setTimeout = function () {};
globalThis.clearTimeout = function () {};

const doc = new Document();
globalThis.document = doc;
globalThis.Image = Image;

globalThis.self = {
  acorn: acorn,
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
