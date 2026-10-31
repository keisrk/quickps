import { Document, HTMLElement } from 'nodom.js';
import * as acorn from 'acorn.js';

HTMLElement.prototype.getContext = function (toodee) {
  return {
    save: function () {},
    restore: function () {}
  }
}

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

globalThis.self = {
  acorn: acorn,
  navigator: {
    userAgent: 'QuickJS'
  },
  window: {
    navigator: {
      userAgent: 'QuickJS'
    },
    addEventListener: function () {},
    requestAnimationFrame: function () {},
    cancelAnimationFrame: function () {},
    setInterval: function () {},
    clearInterval: function () {},
    setTimeout: function () {},
    clearTimeout: function () {}
  },
  document: new Document(),
  XMLSerializer: function () {
    this.serializeToString = function (node) {
      return node.outerHTML;
    };
  },
}
