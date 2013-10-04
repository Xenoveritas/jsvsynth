Import("tests.avsi")

# All this does is test to make sure that all the documented script examples
# run and don't crash AviSynth.

JavaScript("""
avs.Subtitle(avs.BlankClip({color:0xFF}), "Hello from JavaScript!")
""")

avsvalue = "Set in AviSynth"
JavaScript("""
avs.Subtitle(avs.BlankClip({color:0xFF}), avs.avsvalue)
""")

avsvalue = 0
JavaScript("""
avs.avsvalue = "Hello " + "AviSynth!";
""")
Subtitle(BlankClip(color=$FF), avsvalue)

Subtitle(BlankClip(), String(JavaScript("12 + 30")))

JavaScript("""
  avs.fromjs = function(str) {
    return "Hello " + str.substring(0,1).toUpperCase() + str.substring(1);
  }
""")
Subtitle(BlankClip(), FromJS("world"))

JavaScript("""
  avs.animfun = function(clip, str, y) {
    return avs.Subtitle(clip, str, {y:y});
  }
""")
c = BlankClip()
Animate(0, 50, "animfun", c,"Scrolling",0, c,"Scrolling",50)
