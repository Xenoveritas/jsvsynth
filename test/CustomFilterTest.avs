Import("tests.avsi")

JavaScript("""
//console.alert("Attach debugger now");
var filter = new AviSynth.Filter(avs.BlankClip());

assert(filter.getFrame != null, "getFrame wasn't set");
assert(filter.child != null, "child isn't available to JavaScript");

filter.getFrame = function(num) {
	var f = this.child.getFrame(num);
	console.log("Got frame: pitch=", f.pitch, "; rowSize=", f.rowSize, "; height=", f.height);
	var data = new Uint32Array(f.data);
	console.log("Retrieved data. There are ", data.length, " bytes")
	for (var i = 0; i < data.length; i++) {
		data[i] = 0xFF33AA66;
	}
	return f;
}

filter;

var yuvFilter = new AviSynth.Filter(avs.BlankClip({pixel_type:"YV12"}));

assert(yuvFilter.getFrame != null, "getFrame wasn't set");
assert(yuvFilter.child != null, "child isn't available to JavaScript");

yuvFilter.getFrame = function(num) {
	var f = this.child.getFrame(num);
	console.log("Got frame");
	var data = new Uint8Array(f.y.data);
	for (var i = 0; i < data.length; i++) {
		data[i] = 128;
	}
	console.log("Modified Y plane");
	data = new Uint8Array(f.u.data);
	for (var i = 0; i < data.length; i++) {
		data[i] = 255;
	}
	console.log("Modified U plane");
	data = new Uint8Array(f.v.data);
	for (var i = 0; i < data.length; i++) {
		data[i] = 0;
	}
	console.log("Modified V plane");
	return f;
}

yuvFilter;
""")
