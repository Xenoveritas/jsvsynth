Import("tests.avsi")

# Sanity checks. Checks to make sure that the environment is "sane"

JavaScript("""
try {
	assert(false);
	throw Error("No assertion error thrown.")
} catch (e) {
	// OK
}
""")

BlankClip()
