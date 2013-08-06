LoadPlugin("..\Debug\jsvsynth.dll")

outside = "Hello JavaScript"
ival = 42

test = JavaScript("""
var foo = 1;
console.log("hello world!")

console.log(avisynth.outside);
console.log("Integer: " + avisynth.ival);
console.log("Pi: " + avisynth.pi);

var blank = avisynth.BlankClip({color:0xFF});
console.log("Created a blank clip: " + blank);

avisynth.fromjs = "hello from JavaScript"
avisynth.outside = "change existing"
blank
""")

c = BlankClip()

test2 = JavaScript("""
console.log("Foo = " + foo);
console.log("Color space = " + avisynth.c.colorSpace);
""")

Subtitle(test, outside)