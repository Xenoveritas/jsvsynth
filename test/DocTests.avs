Import("tests.avsi")

# All this does is test to make sure that all the documented script examples
# run and don't crash AviSynth.

JavaScript("""
avisynth.Subtitle(avisynth.BlankClip({color:0xFF}), "Hello from JavaScript!")
""")

avsvalue = "Set in AviSynth"
JavaScript("""
avisynth.Subtitle(avisynth.BlankClip({color:0xFF}), avisynth.avsvalue)
""")

avsvalue = 0
JavaScript("""
avisynth.avsvalue = "Hello " + "AviSynth!";
""")
Subtitle(BlankClip(color=$FF), avsvalue);

Subtitle(BlankClip(), String(JavaScript("12 + 30")))

JavaScript("""
  avisynth.fromjs = function(str) {
    return "Hello " + str.substring(0,1).toUpperCase() + str.substring(1);
  }
""")
Subtitle(BlankClip(), FromJS("world"))

JavaScript("""
  avisynth.animfun = function(clip, str, y) {
    return avisynth.Subtitle(clip, str, {y:y});
  }
""")
c = BlankClip()
Animate(0, 50, "animfun", c,"Scrolling",0, c,"Scrolling",50)
