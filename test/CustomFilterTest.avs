Import("tests.avsi")

JavaScript("""
var filter = new AviSynth.Filter(avs.BlankClip());

console.log(filter.getFrame);

assert(filter.getFrame != null, "getFrame wasn't set");
assert(filter.child != null, "child isn't available to JavaScript");

filter;
""")
