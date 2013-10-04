Import("tests.avsi")

# Tests bridging between JavaScript and AviSynth

val_b = true
val_i = 42
val_f = 3.14159
val_s = "hello world"

function val_func(string s) {
    return "hello " + s
}

JavaScript("""
var b = avs.val_b;
assert(typeof b == "boolean", "Boolean not wrapped as boolean");
assert(b === true, "Boolean value is wrong");
var i = avs.val_i;
assert(typeof i == "number", "Int not wrapped as number");
assert(i === 42, "Int value is wrong");
var f = avs.val_f;
assert(typeof f == "number", "Float not wrapped as number");
assert(Math.abs(f - 3.14159) < 0.000005, "Float value did not enter (got " + f + ")");
var s = avs.val_s;
assert(typeof s == "string", "String not wrapped as string");
assert(s === "hello world", "String value is wrong");
var func = avs.val_func;
// assert(typeof func == 'function', 'Function not wrapped as function');
// For now, just verify we can invoke it at all
assert(func("JavaScript") == "hello JavaScript", "Function not invoked")


// And now bridge the other way
avs.val_b = false
avs.val_i = 61
avs.val_f = 2.7182
avs.val_s = "goodbye test"
""")

Assert(val_b == false, "Boolean value did not exit")
Assert(val_i == 61, "int value did not exit")
Assert(abs(val_f - 2.7182) < 0.000005, "Float value did not exit (got " + String(val_f) + ")")
Assert(val_s == "goodbye test", "String value did not exit")

BlankClip()
