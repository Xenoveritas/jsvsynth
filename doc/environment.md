JavaScript Environment
======================

This document describes the contents of the JavaScript environment avaiable
when running within JSVSynth.

AviSynth
--------

The `AviSynth` object is effectively the "namespace" for various AviSynth
functions and contains the following:

* `AviSynth.Filter` - constructor for creating custom AviSynth filters
* `AviSynth.PLANAR_Y` - constant for the Y plane, see "VideoFrame"
* `AviSynth.PLANAR_U` - constant for the U plane, see "VideoFrame"
* `AviSynth.PLANAR_V` - constant for the V plane, see "VideoFrame"
* `AviSynth.PLANAR_ALIGNED` - flag for aligned planes
* `AviSynth.PLANAR_Y_ALIGNED` - same as above, but aligned
* `AviSynth.PLANAR_U_ALIGNED`
* `AviSynth.PLANAR_V_ALIGNED`
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

`avs` provides access to the AviSynth functions and variables. **Note**: for the
most part, this delegates to `AviSynth.functions` and `AviSynth.variables`.

Currently this has functions mask variables. If a variable shares the name of a
function, the function will be returned.

If you need to access a specific type, use either the `AviSynth.functions` or
`AviSynth.variables` collections.

Note that attempting to set a function to the `AviSynth.variables` collection
will instead coerce the value to a string, which probably isn't what you'd like.

Setting a non-function to the `AviSynth.functions` on the other hand will raise
an error.
