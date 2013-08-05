In order to build this plugin, you'll require v8, which is not included.

At one point I was hoping to include just the V8 binaries so that this wouldn't
be needed, but they're so large that it's currently easier just to make you grab 
the entire thing. Plus you need to build with gyp anyway.

Using the Plugin
----------------

With a compiled version of the plugin, using it from within AviSynth is fairly
simple:

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