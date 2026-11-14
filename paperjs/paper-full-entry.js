import paper from 'paper-full.js';

// Hook View.prototype.draw to invoke canvas._onDraw if attached
if (paper && paper.View && paper.View.prototype) {
  const origDraw = paper.View.prototype.draw;
  paper.View.prototype.draw = function () {
    const res = origDraw.apply(this, arguments);
    const canvas = this._element;
    if (canvas && typeof canvas._onDraw === 'function') {
      canvas._onDraw(canvas);
    }
    return res;
  };
}

export default paper;
