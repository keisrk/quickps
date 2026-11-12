import paper from 'paper-core.js';
import { defineScope } from './preamble-core.js';

// Hook View.prototype.draw to invoke canvas._onDraw if attached
if (paper && paper.View && paper.View.prototype) {
  const origDraw = paper.View.prototype.draw;
  paper.View.prototype.draw = function () {
    origDraw.apply(this, arguments);
    const canvas = this._element;
    if (canvas && typeof canvas._onDraw === 'function') {
      canvas._onDraw(canvas);
    }
  };
}

export default paper;
export { defineScope };
export const {
  Point,
  Size,
  Rectangle,
  Matrix,
  Line,
  Path,
  CompoundPath,
  Segment,
  Curve,
  PathItem,
  Shape,
  Item,
  Group,
  Layer,
  Raster,
  PlacedItem,
  HitResult,
  TextItem,
  PointText,
  Color,
  Gradient,
  GradientColor,
  Style,
  PaperScope,
  Project,
  View,
  Tool,
  Key,
  Numerical,
  Base
} = paper;
