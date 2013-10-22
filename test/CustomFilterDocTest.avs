Import("tests.avsi")

# This file runs through the samples in the javascript_filter.md file to make
# sure they actually work.

sample1 = JavaScript("""
    var filter = new AviSynth.Filter(avs.BlankClip());
    filter.getFrame = function(frameNumber) {
        var frame = this.child.getFrame(frameNumber);
        var context = frame.getContext('simple');
        context.fillRect(0xFF0000FF, 10, 10, 20, 20);
        return frame;
    };
    filter;
""")

sample2 = JavaScript("""
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
""")

sample1.ChangeFPS(sample2).AudioDub(sample2) ++ sample2
