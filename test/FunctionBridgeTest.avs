Import("tests.avsi")

JavaScript("""
function join(separator) {
    var res = "";
    for (var i = 1; i < arguments.length; i++) {
        if (i > 1)
            res += separator;
        res += arguments[i];
    }
    return res;
}
join.avsSignature = "ss+";
avs.join = join;
function join_no_expand(separator, values) {
    var res = "";
    for (var i = 0; i < values.length; i++) {
        if (i > 0)
            res += separator;
        res += values[i];
    }
    return res;
}
join_no_expand.avsSignature = "ss+";
join_no_expand.avsExpandArrays = "never";
avs.join_no_expand = join_no_expand;
""")

str = join(" ", "Peter", "picked", "a", "peck", "of", "pickled", "peppers")
Assert(str == "Peter picked a peck of pickled peppers", "Join failed, got " + String(str))

str2 = join_no_expand(" ", "She", "sells", "sea", "shells")
Assert(str2 == "She sells sea shells", "Join failed, got " + String(str2))

Subtitle(BlankClip(), str)
Subtitle(str2, y=40)
