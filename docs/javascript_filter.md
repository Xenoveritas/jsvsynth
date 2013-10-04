AviSynth.Filter
===============

The `AviSynth.Filter` class is used to create a JavaScript-based filter. At
present, you have to wrap an existing clip to use as a source filter. A future
version will allow you to write a filter that acts as a clip source.

Constructors
------------

* `AviSynth.Filter(base, [options])` - create a filter using the given base clip
  as its source
* `AviSynth.Filter(width, height, [options])` - **not implemented yet** - this
  is the constructor that will be used to create a source filter. Currently it
  simply does not exist.

Both constructors have an optional "options" object that can contain the
following fields:

* `frameCount` - set the number of frames this filter generates
* `frameRate` - set the frame rate reported by the filter. At present this has
  to be an array with two values, the frame rate numerator and denominator,
  because I'm too lazy to write code to come up with a ratio for a given
  decimal.
* `getFrame` - a function that generates frames

Fields
------

Generally speaking the fields directly mirror the AviSynth clip information
methods. All the fields are read-only.

* `child` - the only `AviSynth.Filter`-specific field, this is the child clip
  that was given to the constructor, if that constructor was used. Otherwise,
  it's `null`. (Currently this is always available, as the "other" constructor
  is never used.)
* `width` - gets the width of the clip
* `height` - gets the height of the clip
* `frameCount` - the number of frames available
* `frameRate` - the frame rate as a floating point number. However, a frame rate
  isn't really stored as a floating point number for a variety of reasons.
  Instead, it's stored as a ratio, and the following three properties get the
  "real" frame rate. This value is simply
  `frameRateNumerator / frameRateDenominator`.
* `frameRateNumerator` - the numerator of the frame rate
* `frameRateDenominator` - the denominator of the frame rate
* `frameRatio` - a JavaScript only property, the above two fields as an array
  (basically `[ frameRateNumerator, frameRateDenominator ]`)
* `audioRate`
* `audioLength`
* `audioLengthF`
* `audioChannels`
* `audioBits`
* `isAudioFloat`
* `isAudioInt`
* `isInterleaved` - if the color data is interleaved
* `isPlanar` - if the color data is planar
* `isRGB` - if the color data is RGB (either RGB24 or RGB32)
* `isRGB24` - if the color data is RGB24
* `isRGB32` - if the color data is RGB32
* `isYUV` - if the color data is YUV
* `isYUY2` - if the color data is YUY2
* `isYV12` - if the color data is YV12
* `isFieldBased` - if the clip is field based (interlaced)
* `isFrameBased` - if the clip is frame based
* `hasAudio` - if the clip has audio
* `hasVideo` - if the clip has video
* `colorSpace` - another JavaScript only property. This gets the colorspace of
  the clip as a string. (For now. A future version is likely to get a colorspace
  object instead.)

Functions
---------

* `getFrame(frameNumber)` - the function used to generate frames. If you used
  the `AviSynth.Filter(base)` constructor, this defaults to
  `function(frameNumber) { return this.child.getFrame(frameNumber); }`.

Implementing the `getFrame(frameNumber)` function allows you to do whatever you
want to do within the filter.
