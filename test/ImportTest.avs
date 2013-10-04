Import("tests.avsi")

ImportJS("ImportTest.js")

JavaScript("""
assert(foo == 42 && bar == 61 && baz == 108, "Import failed")
""")

BlankClip()
