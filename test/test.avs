LoadPlugin("..\Debug\jsvsynth.dll")

test = JavaScript("""
var foo = 1;
console.log("hello world!")
"hello from JavaScript"
""")

test2 = JavaScript("""
console.log("Foo = " + foo);
""")

BlankClip()
Subtitle(last, test)