JavaScript Environment
======================

This document describes the contents of the JavaScript environment avaiable
when running within JSVSynth.

AviSynth
--------

The `AviSynth` object is effectively the "namespace" for various AviSynth
functions and contains the following:

* `AviSynth.ColorSpace` - a collection that describes the existing colorspaces
  in AviSynth.
* `AviSynth.createVideoFrame(colorspace, width, height)` - function for creating
a raw frame.
* `AviSynth.Filter` - constructor for creating custom AviSynth filters, see
  the "Filter" section below for details
* `AviSynth.PLANAR_Y` - constant for the Y plane, see "VideoFrame"
* `AviSynth.PLANAR_U` - constant for the U plane, see "VideoFrame"
* `AviSynth.PLANAR_V` - constant for the V plane, see "VideoFrame"
* `AviSynth.VideoFrame` - class that provides direct access to video frame data.
Instances can be created directly in order to create blank frames or be pulled
from an existing clip.
* `AviSynth.functions` - a collection of AviSynth functions. This allows
accessing and setting AviSynth functions from JavaScript. Note that functions
and variables have different namespaces in AviSynth, while they share a
namespace in JavaScript. This only access functions. Also note that the
function and variable namespaces in AviSynth are case-insensitive. This
collection reflects that - `AviSynth.functions['foo']` is the same as
`AviSynth.functions['FOO']`.
* `AviSynth.variables` - a collection of AviSynth variables. This allows
accessing and setting AviSynth variables from JavaScript. See the above note.

avs
---

`avs` provides access to both AviSynth functions and variables. In AviSynth,
functions and variables exist in separate namespaces. Which one gets used is
based on the syntax. This means that the following code is perfectly valid
AviSynth:

```avisynth
foo = 1
function foo() {
    return 2
}
```

Both values still exist and can be used. However, in JavaScript, there is no
separation of functions and variables. This means that there's no (good) way to
offer both a function and a value back from AviSynth.

Instead, currently, if a function exists, it will "mask" the variable of the
same name. This means that while you can do something like:

    blankclip = "Hello"
    JavaScript("""
        console.log(AviSynth.variables.blankclip + " World!")
        avs.BlankClip()
    """)

And access both the variable and function despite them having the same name.

Note that attempting to set a function to the `AviSynth.variables` collection
will instead coerce the value to a string, which probably isn't what you'd like.

Setting a non-function to the `AviSynth.functions` on the other hand will raise
an error.

Filter
------

`AviSynth.Filter` lets you write a filter using JavaScript:

```javascript
var myfilter = new AviSynth.Filter(avs.BlankClip());
myfilter.getFrame = function(frameNumber) {
    return this.child.getFrame(frameNumber);
};
```

At its most basic, an AviSynth filter is something that produces frames. In the
above example, frames from a `BlankClip()` are being passed straight through to
the other end.

### Things that will be done ... later

* Allow a filter to be created without a "child" filter. At present, you always
  have to wrap an existing filter, which is used to determine what fields are
  present.

VideoFrame
----------

A video frame can be produced by a clip using the `getFrame(framenumber)`
function. (This can be used on literally any clip, and, in theory at least, this
should always produce results.)

The video frame provides raw access to the pixel data for a frame. There are two
different types of video frame objects that can be returned: one for interleaved
access, and one for planar access. The planar access one is slightly weirder
because you access the Y, U, and V frames seperately.

**Note:** Once a frame has been returned from a filter, it will be "neutered"
and any remaining references to it will no longer be able to access data. If you
access a frame but do not return it to the filter, you should manually
`release()` it when you're done with it. (If you don't, V8 should eventually
free the video frame when it gets garbage collected. Unless you hold on to a
reference. Don't do that.)

ColorSpace
----------

The ColorSpace object contains the following constants:

* `BGR24` - 24 bit RGB
* `BGR32` - 32 bit RGB
* `YUY2` - YUY2
* `YV12` - YV12
* `I420` - Essentially YV12, but ... different. This is essentially a "secret"
internal AviSynth colorspace that isn't reflected by the "standard" AviSynth
scripting langauge. `IsYV12(clip)` will return `True` for both `YV12` and
`I420` clips. However, they *are* different, and custom filter writers will need
to know the difference. Which I'm unclear on. Glad to be of help.

Note that these are all based on internal AviSynth values and reflect that.
Technically AviSynth doesn't have RGB (in that they don't pack the bytes in
that order).
