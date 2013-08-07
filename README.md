JSVSynth
========

JSVSynth is a plugin to AviSynth that allows the use of JavaScript within
AviSynth scripts. It uses [Google's V8 JavaScript
engine](https://developers.google.com/v8/) (the same JavaScript
engine that Chrome uses) to run JavaScript. This means that the pure JavaScript
code will run with the same speed that Chrome's pure JavaScript does. (The
scripting bridge provided by the plugin may not be quite as fast.)

JSVSynth is licensed under the GPLv3. Note that, just like AviSynth being under
the GPL, this does not require scripts written using JSVSynth being released
under the GPLv3.

Using the Plugin
----------------

With a compiled version of the plugin (see Building the Plugin below), using it
from within AviSynth is fairly simple:

    LoadPlugin("jsvsynth.dll")

You can then use AviSynth's multi-line string blocks to write your actual
JavaScript code:

    JavaScript("""
    avisynth.Subtitle(avisynth.BlankClip({color:0xFF}), "Hello from JavaScript!")
    """)

The JavaScript object `avisynth` offers access to any AviSynth variable or
function from within JavaScript:

    avsvalue = "Set in AviSynth"
    JavaScript("""
    avisynth.Subtitle(avisynth.BlankClip({color:0xFF}), avisynth.avsvalue)
    """)

You can almost do the reverse, but with one caveat:

    avsvalue = 0
    JavaScript("""
    avisynth.avsvalue = "Hello " + "AviSynth!";
    """)
    Subtitle(BlankClip(color=$FF), avsvalue);

The value has to be set to something before it will work. Otherwise, even though
the value will be set, AviSynth will give an "I don't know what avsvalue means"
error for the variable.

I'm not entirely sure whether or not this is a problem with AviSynth or is
instead something I'm unclear about within the AviSynth API.

### Calling AviSynth Filters from JavaScript

For the most part, this works like you'd expect: invoke a function using the
exact same values you would in AviSynth. However, things work slightly
differently when using JavaScript than they would in AviSynth.

1. At present, it's not possible to do something like `clip.Trim(50)`. A future
   update will likely attempt to make code like that possible.
2. Named variables are handled as an object, as demonstrated by the
   `avisynth.BlankClip({color:0xFF})` above. This value must always be the
   **last** value given. Otherwise, it'll be translated into a string (likely
   `[object Object]`).
3. Conversions from JavaScript don't take into account the signature of the
   function being called. (As the signature can't be retrieved by plugins
   anyway.)
4. `last` is not a thing in JavaScript. Calling a filter without specifying a
   clip won't automatically cause `last` to be used.

### Other Weird Caveats

AviSynth is case insensitive, and the `avisynth` object will reflect that:

`avisynth.blankclip()` is the same as `avisynth.BlankClip()`.

JavaScript is not: `avisynth.blankclip()` is **not** the same as
`AviSynth.blankclip()`!

This is one of the reasons why you can't use `for(var i in avisynth)` to iterate
over variables in AviSynth. (The primary reason is that the AviSynth API offers
absolutely no way to pull the list of variables in any case.)

Building the Plugin
-------------------

In order to build this plugin, you'll require V8, which is not included. (At one
point I was hoping to include just the V8 binaries, but they're large enough and
V8 is a fast enough moving target that it just isn't feasible.)

In any case, use SVN to download the V8 trunk into "v8":

    svn http://v8.googlecode.com/svn/trunk v8

Note that while using git is "recommended," the git instructions are for
accessing the "bleeding edge" branch, while all we want is the "stable" branch,
which is the SVN trunk branch.

Once you have V8 downloaded, you can build it by following the [V8 building with
GYP instructions](http://code.google.com/p/v8/wiki/BuildingWithGYP). Make sure
you note the bit about downloading the Chromium-provided Cygwin, as the Visual
Studio build **will not work without it**.

Note that if you're using a more recent version of Visual Studio (or is it just
the express versions?) you'll need to use `MSBuild` instead of `devenv` to build
the project from the command line. The command is similar:

    MSBuild build/all.sln /p:Configuration=Release 

If you're planning on building the debug version of the plugin, you'll want to
use `/p:Configuration=Debug` instead.

Once V8 is built, you should be able to build everything else using the
`jsvsynth.sln`.