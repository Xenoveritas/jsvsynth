/** @license
 * This file is part of JSVSynth.
 *
 * JSVSynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JSVSynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JSVSynth.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file serves both to document the API calls available via AviSynth and
 * as an "externs" file for the Google Closure Compiler.
 */

/**
 * A "special" object that provides access to the AviSynth environment.
 * Essentially a collection that first searches for functions from
 * {@link AviSynth.functions} and, if it finds nothing there, then searches
 * {@link AviSynth.variables}. This class is read/write: you can both read
 * functions and variables from AviSynth, and write functions and variables back
 * to AviSynth.
 * <p>
 * Note: AviSynth variables are case-insensitive, meaning that
 * <code>avs.foo</code> and <code>avs.Foo</code> refer to the same value.
 * <p>
 * Note: you cannot use <code>for (var v in avs) {}</code> to iterate over
 * variables in AviSynth, you can only get and set values through this
 * collection.
 * @type {Object[]}
 */
var avs = {};

/**
 * The primary namespace for AviSynth-related builtins.
 * @namespace
 */
function AviSynth() { }

/**
 * A collection of functions in the AviSynth scripting environment. AviSynth
 * has functions and variables occupy separate namespaces, so this allows you
 * to limit access to solely functions. It provides
 * read/write access like {@linkcode avs}, with the exception that you can
 * only export functions to AviSynth in this way.
 * <p>
 * Note: AviSynth functions are case-insensitive, meaning that
 * <code>AviSynth.functions.AVISource</code> and
 * <code>AviSynth.functions.avisource</code> refer to the same function.
 * <p>
 * Note: you cannot use <code>for (var f in AviSynth.functions) {}</code> to
 * iterate over functions in AviSynth, you can only get and set functions
 * through this collection.
 * @type {Object.<string, function>}
 */
AviSynth.functions = {};

/**
 * A collection of variables in the AviSynth scripting environment. AviSynth
 * has functions and variables occupy separate namespaces, so this allows you
 * to access a variable that has been "masked" by a function. It provides
 * read/write access like {@linkcode avs}, with the exception that functions
 * cannot be exported to AviSynth in this way.
 * <p>
 * Note: AviSynth variables are case-insensitive, meaning that
 * <code>AviSynth.variables.foo</code> and <code>AviSynth.variables.Foo</code>
 * refer to the same value.
 * <p>
 * Note: you cannot use <code>for (var v in AviSynth.variables) {}</code> to
 * iterate over variables in AviSynth, you can only get and set values through
 * this collection.
 * @type {Object.<string, *>}
 */
AviSynth.variables = {};

/**
 * An array of available color spaces within AviSynth. Each object is a
 * {@linkcode AviSynth.ColorSpace} object.
 * @type {Array.<AviSynth.ColorSpace>}
 */
AviSynth.ColorSpaces = [];

/**
 * A color space. Note that while you can construct new instances of this
 * object, there is no point in doing so, as only the built-in color space
 * instances actually do anything.
 * <p>
 * (Actually, strictly speaking, the above is not true: any place that JSVSynth
 * requires a <code>AviSynth.ColorSpace</code>, it's actually looking for either
 * a string or an object which contains a property called <code>name</code> that
 * contains the name of the color space. So you can in fact create a "new"
 * ColorSpace object and, if you supply the name of an existing color space, it
 * will, in fact, "work." But there's no point to doing this.)
 * @constructor
 */
AviSynth.ColorSpace = function() { };

/**
 * The name of the colorspace. This is a string, and is one of:
 * <ul>
 * <li><code>RGB32</code></li>
 * <li><code>RGB24</code></li>
 * <li><code>YUY2</code></li>
 * <li><code>YV12</code></li>
 * </ul>
 * @readonly
 * @type {string}
 */
AviSynth.ColorSpace.prototype.name = "";

/**
 * The RGB32 color space.
 * @type {AviSynth.ColorSpace}
 */
AviSynth.ColorSpace.RGB32 = new AviSynth.ColorSpace();

/**
 * The RGB24 color space.
 * @type {AviSynth.ColorSpace}
 */
AviSynth.ColorSpace.RGB24 = new AviSynth.ColorSpace();

/**
 * The YUY2 color space.
 * @type {AviSynth.ColorSpace}
 */
AviSynth.ColorSpace.YUY2 = new AviSynth.ColorSpace();

/**
 * The YV12 color space.
 * @type {AviSynth.ColorSpace}
 */
AviSynth.ColorSpace.YV12 = new AviSynth.ColorSpace();

/**
 * This functions as the base class for both {@link AviSynth.Clip} and
 * {@link AviSynth.Filter}. It provides access to the video information for a
 * clip. (Clips and filters are effectively the same thing.)
 * <p>
 * Most of the field names are based on the clip property getters that
 * AviSynth provides.
 * @constructor
 */
AviSynth.VideoInfo = function() { };

AviSynth.VideoInfo.prototype = {
	/**
	 * The width of the video frame.
	 * @type number
	 * @readonly
	 */
	width: 0,
	/**
	 * The height of the video frame.
	 * @type number
	 * @readonly
	 */
	height: 0,
	/**
	 * The number of frames available in the video clip.
	 * @type number
	 * @readonly
	 */
	frameCount: 0,
	/**
	 * The frame rate of the clip as a floating point number. Note that it isn't
	 * stored that way internally - the actual frame rate is a ratio stored as
	 * integers. Those are available via the <code>frameRateNumerator</code> and
	 * <code>frameRateDenominator</code> members.
	 * @type number
	 * @readonly
	 */
	frameRate: 0.0,
	/**
	 * The numerator of the frame rate ratio.
	 * @type number
	 * @readonly
	 */
	frameRateNumerator: 0,
	/**
	 * The denominator of the frame rate ratio.
	 * @type number
	 * @readonly
	 */
	frameRateDenominator: 0,
	/**
	 * The audio rate of the clip in hertz.
	 * @type number
	 * @readonly
	 */
	audioRate: 0,
	/**
	 * The total number of samples in the audio portion of the clip.
	 * @type number
	 * @readonly
	 */
	audioLength: 0,
	/**
	 * The total number of samples in the audio portion of the clip. This is
	 * literally identical to the <code>audioLength</code> member and is
	 * included only for symmetry with the AviSynth scripting language.
	 * @type number
	 * @readonly
	 */
	audioLengthF: 0,
	/**
	 * The number of audio channels.
	 * @type number
	 * @readonly
	 */
	audioChannels: 0,
	/**
	 * The number of bits used in a single audio sample.
	 * @type number
	 * @readonly
	 */
	audioBits: 0,
	/**
	 * If the audio samples are floating point.
	 * @type boolean
	 * @readonly
	 */
	isAudioFloat: false,
	/**
	 * If the audio samples are integers.
	 * @type boolean
	 * @readonly
	 */
	isAudioInt: false,
	/**
	 * If the video is stored in a planar format.
	 * @type boolean
	 * @readonly
	 * @see AviSynth.PlanarVideoFrame
	 */
	isPlanar: false,
	/**
	 * If the color format is RGB. (RGB24 or RGB32.)
	 * @type boolean
	 * @readonly
	 */
	isRGB: false,
	/**
	 * If the color format is RGB24.
	 * @type boolean
	 * @readonly
	 */
	isRGB24: false,
	/**
	 * If the color format is RGB32.
	 * @type boolean
	 * @readonly
	 */
	isRGB32: false,
	/**
	 * If the color format is YUV. (YUY2 or YV12.)
	 * @type boolean
	 * @readonly
	 */
	isYUV: false,
	/**
	 * If the color format is YUY2.
	 * @type boolean
	 * @readonly
	 */
	isYUY2: false,
	/**
	 * If the color format is YV12.
	 * @type boolean
	 * @readonly
	 */
	isYV12: false,
	/**
	 * Assuming the clip is interlaced (there's no flag for that in AviSynth),
	 * this indicates that both the top and bottom lines are stored in a single
	 * frame.
	 * @type boolean
	 * @readonly
	 */
	isFieldBased: false,
	/**
	 * Assuming the clip is interlaced (there's no flag for that in AviSynth),
	 * this indicates that the top and bottom lines are stored in separate
	 * frames.
	 * @type boolean
	 * @readonly
	 */
	isFrameBased: false,
	/**
	 * If the video is stored in an interleaved format.
	 * @type boolean
	 * @readonly
	 * @see AviSynth.InterleavedVideoFrame
	 */
	isInterleaved: false,
	/**
	 * Whether or not the clip has audio.
	 * @type boolean
	 * @readonly
	 */
	hasAudio: false,
	/**
	 * Whether or not the clip has video.
	 * @type boolean
	 * @readonly
	 */
	hasVideo: false,
	/**
	 * The exact colorspace the clip is in. (NOTE: A future version will likely
	 * replace the string return value with the corresponding
	 * {@linkcode AviSynth.ColorSpace} object.)
	 * @type string
	 * @readonly
	 */
	colorSpace: "",
	/**
	 * The frame rate as a ratio. This is simply
	 * <code>[frameRateNumberator, frameRateDenominator]</code>
	 * @type Array.<number>
	 * @readonly
	 */
	frameRatio: [ 0, 0 ]
};

/**
 * A Clip from AviSynth. This can never be created directly, they can
 * only be retrieved as results from AviSynth functions or directly from the
 * AviSynth environment.
 * To write a custom filter, extend {@link AviSynth.Filter}.
 * @constructor
 * @extends AviSynth.VideoInfo
 * @see avs
 * @see AviSynth.Filter
 */
AviSynth.Clip = function() { };

AviSynth.Clip.prototype = new AviSynth.VideoInfo();

/**
 * Gets a single frame from the clip, assuming this clip has video.
 * @param {number} frame
 *                  the frame to get (0-index, 0 is the first frame)
 * @return {AviSynth.VideoFrame}
 *                  a single video frame from the clip
 */
AviSynth.Clip.prototype.getFrame = function(frame) { };

/**
 * A JavaScript-created filter. This allows you to write a custom filter using
 * JavaScript. Replace the {@link AviSynth.Filter#getFrame} function with your
 * own implementation to create your own filter.
 * <p>
 * Note that because the child is <em>required</em> to be present (the video
 * information must be populated) - you can't extend this class in JavaScript.
 * <p>
 * FIXME: it should be possible to extend this class. This will probably be done
 * by setting "defaults" for the video information. The entire set of video
 * information required is basically the same as those taken by AviSynth's
 * <code>BlankClip</code> filter: int "length", int "width", int "height", string "pixel_type",
   float "fps", int "fps_denominator", int "audio_rate", int "channels",
   string "sample_type", int "color", int "color_yuv"
 * @constructor
 * @param {AviSynth.VideoInfo} child
 *                  the source filter to get frames from
 * @extends AviSynth.VideoInfo
 * @see avs
 * @see AviSynth.Filter
 */
AviSynth.Filter = function(child) { };

AviSynth.Filter.prototype = new AviSynth.VideoInfo();

/**
 * The child source of frames. If this was created without a child clip, this
 * will return {@code null}.
 * @type AviSynth.VideoInfo
 * @readonly
 */
AviSynth.Filter.prototype.child = null;

/**
 * Gets a single frame from the clip, assuming this clip has video. Unlike
 * the AviSynth.Clip version, this is pure JavaScript, meaning you can replace
 * this function with whatever you like.
 * <p>
 * The default implementation is simply:
 * <p>
 * <code>function(n) { return this.child.getFrame(n); }</code>
 * @param {number} frame
 *                  the frame to get (0-index, 0 is the first frame)
 * @return {AviSynth.VideoFrame}
 *                  a single video frame from the clip
 */
AviSynth.Filter.prototype.getFrame = function(frame) { };

/**
 * A video frame. At present, you can't construct video frames directly. This
 * may change at a future point in time.
 * <p>
 * All video frames accessed from AviSynth are actually either a
 * {@linkcode AviSynth.InterleavedVideoFrame} (for RGB24/RGB32/YUY2) or a
 * {@linkcode AviSynth.PlanarVideoFrame} (for YV12).
 * @constructor
 */
AviSynth.VideoFrame = function() { };

AviSynth.VideoFrame.prototype = {
	/**
	 * Release the frame, indicating that the frame data is no longer useful.
	 * This will allow AviSynth to "reclaim" the frame.
	 */
	release: function() { },
	/**
	 * Indicates whether the frame data is interleaved (RGB, YUY2).
	 *
	 * @type {boolean}
	 * @readonly
	 */
	interleaved: false,
	/**
	 * Indicates whether the frame data is planar (YV12).
	 *
	 * @type {boolean}
	 * @readonly
	 */
	planar: false,
	/**
	 * Gets a rendering context, allowing you to draw directly onto the frame.
	 *
	 * @param {string} type
	 *                  the type of rendering context, currently this must be
	 *                  the string <code>"simple"</code> indicating that a
	 *                  {@link AviSynth.SimpleRenderingContext} should be
	 *                  returned. <code>"2d"</code> to get an HTML5 canvas style
	 *                  context is planned, <code>"webgl"</code> would sure be
	 *                  cool but don't count on it.
	 * @return {AviSynth.RenderingContext}
	 *                  a rendering context to render on the frame, or
	 *                  <code>null</code> if the requested context is not
	 *                  available
	 */
	getContext: function(type) { return null; }//,
	/* *
	 * Convert a frame to a different colorspace.
	 * @future
	 * @param {string} colorspace
	 *                  the colorspace to convert to
	 * /
	convertToColorspace: function(colorspace) { }*/
};

/**
 * Interface for accessing actual pixel data.
 * @mixin
 */
AviSynth.VideoFrameData = function() { };

AviSynth.VideoFrameData.prototype = {
	/**
	 * The data buffer. You'll want to wrap this with one of the more specific
	 * typed-array views.
	 * @type {ArrayBuffer}
	 * @readonly
	 */
	data: null,
	/**
	 * The pitch: the number of bytes between the start of one row and the start
	 * of the next.
	 * @type {number}
	 * @readonly
	 */
	pitch: 0,
	/**
	 * The row size: the number of bytes after the start of the row that are
	 * actually visible in the final frame.
	 * @type {number}
	 * @readonly
	 */
	rowSize: 0,
	/**
	 * The width of the frame in pixels. In the case of planar data,
	 * this may not be what you're expecting due to chroma subsampling!
	 * @type {number}
	 * @readonly
	 */
	width: 0,
	/**
	 * The height of the frame in pixels. In the case of planar data,
	 * this may not be what you're expecting due to chroma subsampling!
	 * @type {number}
	 * @readonly
	 */
	height: 0,
	/**
	 * The number of <em>bits</em> in a single pixel.
	 * @type {number}
	 * @readonly
	 */
	bitPerPixel: 0,
	/**
	 * The number of <em>bytes</em> in a single pixel.
	 * @type {number}
	 * @readonly
	 */
	bytesPerPixel: 0//,
	/* *
	 * Get a single pixel value. (This is difficult due to chroma subsampling
	 * and will be wonky under YUV frames.)
	 * @future
	 * /
	getPixel: function(x, y) { }*/
};

/**
 * This class provides direct access to video frame data that is in an
 * interleaved format.
 * @constructor
 * @extends AviSynth.VideoFrame
 * @extends AviSynth.VideoFrameData
 * @see AviSynth.PlanarVideoFrame
 */
AviSynth.InterleavedVideoFrame = function() { };

AviSynth.InterleavedVideoFrame.prototype = new AviSynth.VideoFrame();

/**
 * This class provides direct access to video frame data that is in an
 * planar format.
 * @constructor
 * @implements AviSynth.VideoFrameData
 * @see AviSynth.InterleavedVideoFrame
 */
AviSynth.PlanarVideoFrame = function() { };

AviSynth.PlanarVideoFrame.prototype = new AviSynth.VideoFrame();

/**
 * The Y plane (luma).
 * @type AviSynth.VideoFrameData
 */
AviSynth.PlanarVideoFrame.prototype.y = null;

/**
 * The U plane (chroma green/blue).
 * @type AviSynth.VideoFrameData
 */
AviSynth.PlanarVideoFrame.prototype.u = null;

/**
 * The V plane (chroma red/green).
 * @type AviSynth.VideoFrameData
 */
AviSynth.PlanarVideoFrame.prototype.v = null;

/**
 * A rendering context capable of writing on a frame. This class cannot be
 * constructed directly, instead use {@link AviSynth.VideoFrame#getContext}.
 * Currently the only supported RenderingContext is
 * {@link AviSynth.SimpleRenderingContext}.
 * @constructor
 */
AviSynth.RenderingContext = function() { };

/**
 * A rendering context capable of writing on a frame. This class cannot be
 * constructed directly, instead use {@link AviSynth.VideoFrame#getContext}.
 * @constructor
 */
AviSynth.SimpleRenderingContext = function() { };

AviSynth.SimpleRenderingContext.prototype = new AviSynth.RenderingContext();

/**
 * Fill a rectange with the given color. For RGB32 clips, the color is a 32-bit
 * value packed <code>ARGB</code>. For YUV clips it will probably be YUV packed
 * in some way, assuming contexts for YUV clips are ever supported. (It's also
 * possibly YUV clips will take in a "special" color object and that numbers
 * will <em>always</em> be RGB.)
 * <p>
 * <strong>Important:</strong> This <strong>does not</strong> composite the
 * rectangle onto the frame, it completely replaces the pixels, including the
 * alpha channel! (This may be changed in the future by providing a "composite"
 * parameter.)
 *
 * @param {number} color
 *                  the color as a number
 * @param {number} x
 *                  the x coordinate in pixels
 * @param {number} y
 *                  the y coordinate in pixels
 * @param {number} width
 *                  the width of the rectangle in pixels
 * @param {number} height
 *                  the height of the rectangle in pixels
 */
AviSynth.SimpleRenderingContext.prototype.fillRect = function(color, x, y, width, height) { };

/**
 * Draw a different frame onto this frame. The colorspaces of the two frames
 * must match.
 * <p>
 * <strong>Important:</strong> This <strong>does not</strong> composite the
 * source image onto this frame, the entire thing (including alpha channel) is
 * copied directly! (This may be changed in the future by providing a "composite"
 * parameter.)
 *
 * @param {AviSynth.VideoFrame} frame
 *                  the other frame to draw onto the context frame
 * @param {number} x
 *                  the x coordinate in pixels
 * @param {number} y
 *                  the y coordinate in pixels
 */
AviSynth.SimpleRenderingContext.prototype.drawImage = function(frame, x, y) { };

/**
 * Draw a portion of a different frame onto this frame. The colorspaces of the
 * two frames must match.
 * <p>
 * <strong>Important:</strong> This <strong>does not</strong> composite the
 * source image onto this frame, the entire thing (including alpha channel) is
 * copied directly!
 *
 * @param {AviSynth.VideoFrame} frame
 *                  the other frame to draw onto the context frame
 * @param {number} sx
 *                  the x coordinate of the part of the source frame to draw in
 *                  pixels
 * @param {number} sy
 *                  the y coordinate of the part of the source frame to draw in
 *                  pixels
 * @param {number} sw
 *                  the width of the part of the source frame to draw in pixels
 * @param {number} sh
 *                  the height of the part of the source frame to draw in pixels
 * @param {number} dx
 *                  the x coordinate to draw to in this frame
 * @param {number} dy
 *                  the y coordinate to draw to in this frame
 */
AviSynth.SimpleRenderingContext.prototype.drawImage = function(frame, sx, sy, sw, sh, dx, dy) { };