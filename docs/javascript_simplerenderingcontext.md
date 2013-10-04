SimpleRenderingContext
======================

A `SimpleRenderingContext` is an API that provides you with "basic" access to
the frame. It allows basic things like filling rectanges and copying frames
from another clip into its frame.

Constructors
------------

You can't create a `SimpleRenderingContext` on your own. Instead, it is
provided using `getContext("simple")` on a
[`AviSynth.VideoFrame`](javascript_videoframe.md).

Fields
------

There are no fields at present.

Functions
---------

* `fillRect(color, x, y, w, h)` - fill a given area with a given color
* `drawFrame(frame, x, y)` - copy a different frame onto this frame
* `drawFrame(frame, sx, sy, sw, sh, dx, dy)` - copy a portion of a different
  frame onto this frame
