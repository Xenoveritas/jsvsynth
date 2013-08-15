Import("tests.avsi")

JavaScript("""
//var ab = new ArrayBuffer(640*480*8);
//console.alert();
var filter = new AviSynth.Filter(avs.BlankClip());

assert(filter.getFrame != null, "getFrame wasn't set");
assert(filter.child != null, "child isn't available to JavaScript");

filter.getFrame = function(num) {
	var f = this.child.getFrame(num);
	console.log("Got frame: pitch=", f.pitch, "; rowSize=", f.rowSize, "; height=", f.height);
	var data = f.data;
	console.log("Retrieved data. There are ", data.length, " bytes")
	for (var i = 0; i < data.length; i++) {
		data[i] = 255;
	}
	return f;
}

filter;
""")
