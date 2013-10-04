Import("loadplugin.avsi")

JavaScript("""
function assert(test, message) {
	if (!(test === true)) {
		throw Error("assertion failed: " + message);
	}
}
""")
