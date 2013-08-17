AviSynth.VideoFrame
===================

The `AviSynth.VideoFrame` class provides access to a video frame.

Constructors
------------

`VideoFrame`s cannot be directly created at present. They can only be obtained
by using the `getFrame(frameNumber)` method on an existing `Clip`. However:

* `AviSynth.VideoFrame(width, height, colorspace)` - **not implemented yet**
  create a new video frame

Fields
------

There are two methods to access video frame data, and the fields available
depend on which on is being used.

* `interleaved` - `true` if the video data is interleaved
* `planar` - `true` if the video data is planar (YV12 only)

If `interleaved` is `true`, (which it will be for RGB and YUY2) then you have:

* `data` - a `ArrayBuffer` that allows access to the frame data - to actually
  use this, you'll need to create a typed array view into it
* `pitch` - the "pitch" (see "accessing a pixel")
* `rowSize` - the "row size" in bytes (see "accessing a pixel")
* `height` - the height, which is simply the height of the frame
* `bitsPerPixel` - the number of **bits** in a single pixel, compare with:
* `bytesPerPixel` - the number of **bytes** in a single pixel, which only works
  for interleaved images (well... see below)

Otherwise, if `planar` is `true`, those fields are instead available inside:

* `y` - the Y plane (luma)
* `u` - the U plane (green/blue)
* `v` - the V plane (red/green)

`ArrayBuffer`  is part of a new JavaScript feature that V8 supports called
[JavaScript typed arrays](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Typed_arrays).
Essentially the `ArrayBuffer` provides an object that represents the data to
JavaScript, while various <code><var>Type</var>Array</code> objects provide
access to the data. (Yes, that link is to the Mozilla documentation. Chrome
doesn't provide documentation directly. However the Mozilla documentation covers
the standard and, as long as you avoid the Mozilla-specific features, covers the
API available through JSVSynth.)

This means that in order to access the pixel data, you need to write it with
an array view. The simplest one is `Uint8Array` which provides byte access to
the pixel data.

However, for RGB32 data, you might find it easier to use `Uint32Array` which
provides you with access to the entire pixels at a time.

In AviSynth, if you have a planar data source, it will be either YV12 or I420
(the difference of which is basically incidental but detectable anyway). This
means that `bitPerPixel` will be 12 and `bytesPerPixel` will be an inaccurate
1.

**NOTE:** Accessing the `data` element at all will force
JSVSynth to make the frame "writeable" which will effectively create an extra
copy of the frame. There is currently no way to get read-only access to a frame.

Functions
---------

* `release()` - release the frame data, preventing future access to the frame.
  After this is called, the length of the data arrays becomes 0 and you will no
  longer be able to access the frame data. **Do not `release()` a frame you're
  returning from `AviSynth.Filter.getFrame()`!**
* `getContext(type)` - based on the [HTML 5 canvas
  API](http://www.whatwg.org/specs/web-apps/current-work/multipage/the-canvas-element.html),
  this provides access to a context that allows drawing on the frame directly.
  At some point you'll be able to do `getContext("2d")` and get something
  similar to a full-fledged
  [`CanvasRenderingContext2D`](http://www.whatwg.org/specs/web-apps/current-work/multipage/the-canvas-element.html#canvasrenderingcontext2d).
  But for now, you're stuck with `getContext("simple")`. **Note:** Currently,
  rendering contexts are limited to RGB32 clips. This *may* change in the
  future for `getContext("simple")` but will likely never change for
  `getContext("2d")`.
* `getPixel(x, y)` [EXPERIMENTAL] - get a pixel

Accessing a Pixel
-----------------

Or, "what's the difference between pitch and rowSize?"

Basically, data that is provided through the various `data` arrays may be
"padded" along the side. The simplest reason is when a clip is cropped - rather
than adjust the frame data, AviSynth just marks the new edges and reuses the
frame data. (There are other reasons for this that have to do with data
alignment, but that's beyond the scope of this page.)

So `rowSize` gets you the number of "visible" bytes in a row, while `pitch` is
the number of bytes you need to move forward to the next row of pixels.

This means accessing a specific pixel is done via:

```javascript
frame.data[y * frame.pitch + x * frame.bytesPerPixel]
```

Techincally, that just gets you the first byte. So, for example, to access all
four channels in an RGB32 clip (RGBA), you'd use:

```javascript
var r = frame.data[y * frame.pitch + x * frame.bytesPerPixel + 2]
var g = frame.data[y * frame.pitch + x * frame.bytesPerPixel + 1]
var b = frame.data[y * frame.pitch + x * frame.bytesPerPixel]
var a = frame.data[y * frame.pitch + x * frame.bytesPerPixel + 3]
```

This makes more sense when you know that the x86 chips AviSynth runs on are
little-endian, and the data is stored in memory as `BGRABGRABGRA...`, meaning
that reading a single 32-bit value from a pixel location winds up creating 32
bits in `ARGB` order, which is the order used for `color` in `BlankClip`.
