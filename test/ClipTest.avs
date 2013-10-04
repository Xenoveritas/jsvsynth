Import("tests.avsi")

# Tests the clip bridge

c = BlankClip()

JavaScript("""
var c = avs.c;
assert(c.width == 640, "Width is wrong (" + c.width + ")");
assert(c.height == 480, "Height is wrong (" + c.height + ")");
console.log(typeof c)
console.log(c.toString())
console.log(c)
assert(c.toString() != '[object Object]', 'toString not implemented');
c
""")
