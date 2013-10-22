AviSynth.Filter
===============

The `AviSynth.Filter` class is used to create a JavaScript-based filter. At
present, you have to wrap an existing clip to use as a source filter. A future
version will allow you to write a filter that acts as a clip source.

Check out the [complete API documentation](api/AviSynth.Filter.html) for
complete details.

The basic idea is that you replace the `getFrame` function with a custom
implementation:

    var filter = new AviSynth.Filter(avs.BlankClip());
    filter.getFrame = function(frameNumber) {
        var frame = this.child.getFrame(frameNumber);
        var context = frame.getContext('simple');
        context.fillRect(0xFF0000FF, 10, 10, 20, 20);
        return frame;
    };

This draws a blue square on a black blank frame. You can actually do some fairly
complicated stuff with this since you can directly access the frame's pixel
data:

    var filter = new AviSynth.Filter(avs.Trim(avs.ColorBars(), 0, 240));
    filter.getFrame = function(frameNumber) {
        var frame = this.child.getFrame(frameNumber);
        var w = frame.width, h = frame.height, pitch = frame.pitch/4;
        var data = new Uint32Array(frame.data);
        for (var y = 0; y < h; y++) {
            for (var x = 0; x < w; x++) {
                data[y*pitch+x] = data[y*pitch+x] ^ 0xFFFFFF;
            }
        }
        return frame;
    };
    filter;

The above example will invert a frame by directly accessing the pixel data and
inverting each pixel. (Note that it actually inverts only the final three bytes,
leaving the alpha channel alone.)
