Import("tests.avsi")

JavaScript("""
//console.alert("Attach debugger now");
var filter = new AviSynth.Filter(avs.BlankClip({length:480-200}));

filter.getFrame = function(num) {
	var f = this.child.getFrame(num);
	var ctx = f.getContext('simple');
	console.log("Context: " + ctx);
	assert(ctx != null, "Unable to get simple context");
	ctx.fillRect(0x80FF66CC, num, num, 200, 200);
	return f;
}

// TODO: Move getting a frame to the test harness, but for now:
var f = filter.getFrame(0);

// Weird thing that should be valid:
var ctx = f.getContext('simple');
f.release();
ctx.fillRect(0xFF, 0, 0, 200, 200);
// That should "work" in that it shouldn't generate an error.
// TODO: Or should it? Should attempting to draw on a released context throw an
// exception?

filter;
""")
