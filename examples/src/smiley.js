/* global document, Path, Point, PointText, Raster, view */

// Background rectangle
new Path.Rectangle({
  point: [0, 0],
  size: [view.size.width, view.size.height],
  fillColor: '#f7fafc'
})

const titleStyle = {
  fontFamily: 'ABeeZee, sans-serif',
  fontSize: 28,
  fillColor: '#1a202c',
  justification: 'center'
}

const labelStyle = {
  fontFamily: 'ABeeZee, sans-serif',
  fontSize: 16,
  fillColor: '#4a5568',
  justification: 'center'
}

/* eslint-disable no-new */
new PointText({
  ...titleStyle,
  point: [view.center.x, 80],
  content: 'Smiley Raster'
})

// Original smiley raster resolved via id ('smiley')
const left = new Raster('smiley')
left.position = new Point(view.center.x - 150, view.center.y + 20)
left.scale(0.8)

new PointText({
  ...labelStyle,
  point: [left.position.x, left.bounds.bottom + 35],
  content: 'Original'
})

// Negative-positive inverted smiley raster resolved via document.getElementById('smiley')
const right = new Raster(document.getElementById('smiley'))
right.position = new Point(view.center.x + 150, view.center.y + 20)
right.scale(0.8)

const imageData = right.getImageData()
const data = imageData.data
for (let i = 0; i < data.length; i += 4) {
  if (data[i + 3] > 0) {
    data[i] = 255 - data[i]
    data[i + 1] = 255 - data[i + 1]
    data[i + 2] = 255 - data[i + 2]
  }
}
right.setImageData(imageData)

new PointText({
  ...labelStyle,
  point: [right.position.x, right.bounds.bottom + 35],
  content: 'Negative (Nega-Posi Inverted)'
})
/* eslint-enable no-new */
