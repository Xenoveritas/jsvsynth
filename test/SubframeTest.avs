Import("tests.avsi")

JavaScript("""

var font = avs.ImageSource("boxy_bold_fixed.png", {"pixel_type":"RGB32"});

var rainbow = new AviSynth.Filter(avs.BlankClip());

rainbow.getFrame = function(num) {
	var f = this.child.getFrame(num);
	var ctx = f.getContext('simple');
	var r = 255;
	var g = num;
	var b = 0;
	ctx.fillRect((r << 16) | (g << 8) | b, 0, 0, 640, 480);
	return f;
}

var filter = new AviSynth.Filter(avs.BlankClip());

function drawText(str, ctx, font, x, y, cw, ch) {
	for (var i = 0; i < str.length; i++) {
		var c = str.charCodeAt(i) - 32;
		if (c >= 0 && c < 96) {
			var cx = c % 16;
			var cy = Math.floor(c / 16);
			ctx.drawImage(font, cx*cw, cy*ch, cw, ch, x, y);
		}
		x += cw;
	}
}

filter.getFrame = function(num) {
	var f = this.child.getFrame(num);
	var fontf = font.getFrame(num);
	var ctx = f.getContext('simple');
	drawText("Hello SubFrames!", ctx, fontf, num, num, 8, 8);
	return f;
};

avs.Layer(rainbow, filter);
""")