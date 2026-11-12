import paper, { defineCanvas, Point, Path, Color, Raster } from './qps.so';

if (!paper) {
  throw new Error('paper is null or undefined');
}
if (paper.PaperScript !== undefined) {
  throw new Error('PaperScript must not be present in core (qps.so)');
}
if (typeof defineCanvas !== 'function') {
  throw new Error('defineCanvas is not a function');
}

let drawCalled = false;
const canvas = defineCanvas({
  width: 300,
  height: 200,
  images: {
    smiley: '../../examples/smiley.png'
  },
  onDraw: (cvs) => {
    drawCalled = true;
    if (typeof cvs.write !== 'function') {
      throw new Error('canvas.write is not a function');
    }
    cvs.write('qps_module_test_out.png');
  }
});

paper.setup(canvas);

// Pure JS operations (no PaperScript vector operator overloading)
const p1 = new Point(50, 50);
const p2 = p1.add(new Point(50, 50));
if (p2.x !== 100 || p2.y !== 100) {
  throw new Error(`Point.add failed: got (${p2.x}, ${p2.y})`);
}

const rect = new Path.Rectangle([20, 20], [100, 80]);
rect.fillColor = new Color(0, 0.5, 1);

// Test Raster using pre-registered image KV from defineCanvas
const raster = new Raster('smiley');
if (!raster || raster.size.width <= 0) {
  throw new Error('Raster failed to load pre-registered image "smiley"');
}
raster.position = new Point(150, 100);

paper.view.draw();

if (!drawCalled) {
  throw new Error('onDraw callback was not triggered during paper.view.draw()');
}

const svg = paper.project.exportSVG({ asString: true });
if (typeof svg !== 'string' || !svg.includes('<svg') || !svg.includes('<path')) {
  throw new Error(`Invalid SVG output: ${svg}`);
}
