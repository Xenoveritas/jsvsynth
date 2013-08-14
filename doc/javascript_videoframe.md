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

* `data` - a `Uint8Array` that allows access to the frame data
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

A `Uint8Array` is a new(ish) JavaScript feature that V8 supports. Since Google
doesn't have JavaScript documentation, check out [the Mozilla JavaScript
documentation of
Uint8Array](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Typed_arrays/Uint8Array).

Basically, though, you can just access the array data exactly like you would any
other array, except the array can only contain unsigned 8-bit integers. This
is the "standard" version, meaning that only the low 8-bits are used. (Which
isn't the way the HTML5 Canvas `ImageData` API works - that "clamps" values to
0-255, so a value less than 0 becomes 0 and a value higher than 255 becomes
255.)

In AviSynth, if you have a planar data source, it will be either YV12 or I420
(the difference of which is basically incidental but detectable anyway). This
means that `bitPerPixel` will be 12 and `bytesPerPixel` will be an inaccurate
1.

Functions
---------

There's only one function available for a VideoFrame:

* `release()` - release the frame data, preventing future access to the frame.
  After this is called, the length of the data arrays becomes 0 and you will no
  longer be able to access the frame data. **Do not `release()` a frame you're
  returning from `AviSynth.Filter.getFrame()`!**

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
