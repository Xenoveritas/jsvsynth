Exporting Functions
===================

Frequently you'll want to make functions declared in JavaScript available to
AviSynth. This is, at its most basic, very easy:

```javascript
avs.foo = function(bar) {
    return Math.PI * bar;
}
```

However, you might want to return a function that takes a bunch of optional
parameters or duplicate the "named value" feature in AviSynth.

This is still done in a similar fashion:

```javascript
var myFunction = function(clip, str, color, x, y) {
    // Some code that checks if values were set and sets defaults, see below
    return avs.Subtitle(clip, str, {color:color, x:x, y:y});
}

myFunction.avsSignature = "c[STRING]s[COLOR]i[X]i[Y]i";
avs.myFunction = myFunction;
```

In this case, the string set to `avsSignature` is used to populate the string.

The signature is simply a string that describes the arguments that an AviSynth
function takes from left to right. Each character represents a single type,
taken from the following pool of types:

* `b` - boolean
* `c` - clip (an AviSynth clip)
* `f` - float (JavaScript number)
* `i` - integer
* `s` - string
* `.` - any type

You can make an argument optional by preceeding it with a name in brackets. In
the above example signature, `"c[STRING]s[COLOR]i[X]i[Y]i"`, the second argument
is an optional argument named `"STRING"` that is a string. The third argument is
an optional argument named `"COLOR"` that is an integer, and so on.

The final aspect of the signature is the number of times values can appear.
In the above examples, all of them take a single value. However, let's take a
quick look at the default signature, used if you don't specify anything: `".*"`

This signature takes any type of value, 0 or more times. The other option is `+`
which takes one or more values. So a function that takes one or more strings
would have a signature of `"s+"`.

--------------------------------------------------------------------------------

**Note:** I have no idea what will happen if you attempt to repeat types and
make an ambiguous signature or combine `+` or `*` with an optional argument.

In other words, I have no idea what `s+s*` would do (as there'd be no way to
tell when one set of strings ends and another begins - yes, this is important,
see the section below on "expanding arrays".

--------------------------------------------------------------------------------


Expanding Arrays
----------------

Now you might expect a function with a signature of `"s+"` to be something like
this:

```javascript
function join(separator) {
    var res = "";
    for (var i = 1; i < arguments.length; i++) {
        if (i > 1)
            res += separator;
        res += arguments[i];
    }
    return res;
}
strcat.avsSignature = "ss+";
avs.join = join;
```

And, from the JavaScript side, this will work as you expected. However, that's
because some "magic" is happening behind the scenes.

When you invoke a function with a `*` or `+` in the signature, all values that
"matched" that set are placed into an AviSynth array. (Yes, AviSynth supports
arrays - but only for passing arguments to functions. This is also the only
place they appear and you can only nest them "two deep" - once for the array of
arguments in the first place, and a second time for any `*` or `+` arguments.)

This means that the following:

```avisynth
strcat(" ", "Peter", "picked", "a", "peck", "of", "pickled", "peppers")
```

Will initially map to the following call:

```javascript
join(" ", [ "Peter", "picked", "a", "peck", "of", "pickled", "peppers" ] )
```

However, this isn't what will get called, and that's due to "array expansion."
If you remember above, the default signature is `".*"`. In order to make most
functions do what you'd expect, if the final argument to a JavaScript function
is an array, JSVSynth will "expand" the array into the arguments list.

This makes most things behave like you'd expect from the JavaScript side.

However, you can still control this, using the `avsExpandArrays` value on the
function itself. This can have any of the following values:

* `"always"` - always expand arrays, in all positions
* `"never"` - never expand arrays, always maintain them
* `"last"` - only expand an array if it's the last value given to the function,
  which is the default
