Import("tests.avsi")

# This script checks to see if the ColorSpace object is set up correctly

JavaScript("""
console.log(AviSynth.ColorSpace);
assert(typeof AviSynth.ColorSpace == 'function', 'Missing ColorSpace object');

var expected = {};

['RGB32', 'RGB24', 'YUY2', 'YV12'].forEach(function(cs) {
	// This is used later to make sure all color spaces are here.
	expected[cs] = false;
	assert(AviSynth.ColorSpace[cs] != null, 'Missing ColorSpace constant for ' + cs);
	assert(AviSynth.ColorSpace[cs].name == cs, 'Wanted ' + cs + ' for name, got ' + AviSynth.ColorSpace[cs].name);
});

assert(AviSynth.ColorSpaces != null, 'Missing ColorSpaces array');
// Make sure the expected colorspaces are in the color spaces array

AviSynth.ColorSpaces.forEach(function(cs) {
	assert(typeof cs == 'object', 'Expected an object, got a ' + cs.toString());
	assert('name' in cs, 'Color space does not include a name field');
	assert(cs.name in expected, 'Unexpected color space ' + cs.name);
	expected[cs.name] = true;
});

var missing = [];
for (var cs in expected) {
	if (expected[cs] === false) {
		missing.push(cs);
	}
}

assert(missing.length == 0, 'Missing color spaces: ' + missing.join(', '));
""")

BlankClip(length=2)
