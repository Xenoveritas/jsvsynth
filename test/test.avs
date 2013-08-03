LoadPlugin("..\Debug\jsvsynth.dll")

outside = "Hello JavaScript"
ival = 42
pi = 3.14159

test = JavaScript("""
var foo = 1;
console.log("hello world!")

console.log(avisynth.outside);
console.log("Integer: " + avisynth.ival);
console.log("Pi: " + avisynth.pi);

avisynth.fromjs = "hello from JavaScript"
avisynth.outside = "change existing"
""")

test2 = JavaScript("""
console.log("Foo = " + foo);
""")

BlankClip()
Subtitle(last, outside)