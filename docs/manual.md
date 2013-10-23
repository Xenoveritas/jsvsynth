JSVSynth User Documentation
===========================

This documentation describes using JSVSynth from within AviSynth. For a
complete list of classes and objects available from JavaScript, [check out the
complete API documentation](api/index.html).

Before you can use the plugin, you'll need to load it. This is a "regular"
AviSynth plugin (not a C plugin), so you use:

```avisynth
LoadPlugin("jsvsynth.dll")
```

Once you've loaded the plugin, you have access to the following new
AviSynth functions:

* `JavaScript(code)`
* `ImportJS(filename)`

These allow you to execute JavaScript code from within AviSynth.

`JavaScript(code)` simply executes the JavaScript code given in `code` while
`ImportJS(filename)` loads the file given by `filename` and executes the
contents as JavaScript.

The JavaScript environment within JSVSynth is very similar to what you'd
expect: things like `Number`, `Date`, and `Math` are all there. However, this
isn't a browser: `window` does not exist and most HTML APIs won't be there. (So
no `XMLHttpRequest` or stuff like that.)

Instead, you have the `AviSynth` and `avs` objects, which provides your bridge
between JavaScript and AviSynth. You can access any AviSynth function or
variable from the `avs` object, just like you would any other object:

```javascript
var baz = avs.foo + avs['bar']
```

AviSynth functions can also be accessed that way. Note that all AviSynth
filters are "functions." If you really wanted to, you can even access the
JavaScript filter provided by JSVSynth through `avs`, although I wouldn't
recommend it. (After all, `eval` is available.)

JSVSynth API
------------

The [full documentation of the AviSynth API](api/index.html) is available as a
set of jsdoc pages. However, some more specific documentation is also available:

* [JavaScript Environment](environment.md) - what's available to JavaScript
  inside an AVS file
* [Exporting Functions](exporting_functions.md) - how to export a JavaScript
  function so that it can be used from AviSynth
* [Writing a JavaScript Filter](javascript_filter.md) - how to write a custom
  AviSynth filter directly in JavaScript
* [SimpleRenderingContext](javascript_simplerenderingcontext.md) - using a
  SimpleRenderingContext to draw on a frame
