LoadPlugin("..\Debug\jsvsynth.dll")

JavaScript("""
function Test() { return "Hello"; }
avisynth.test = Test;
""")

Subtitle(BlankClip(), Test())