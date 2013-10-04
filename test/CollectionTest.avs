Import("tests.avsi")

foo = 1
function foo() {
	return 2
}

JavaScript("""
assert(avs.foo() == 2, "Did not grab function")
assert(AviSynth.functions.foo() == 2, "Function collection failed")
assert(AviSynth.variables.foo == 1, "Variables collection failed")
""")

BlankClip()
