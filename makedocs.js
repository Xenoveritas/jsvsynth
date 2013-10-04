#!/usr/bin/env node

// Quicky script to build our documentation from the Markdown sources and run
// JSDoc if it's available.

var doc_dir = 'docs';
var output_dir = 'build/website';
var css = [ 'bootstrap/css/bootstrap.min.css', 'bootstrap/css/bootstrap-theme.min.css' ];
var jsdoc_api = 'docs/api/jsvsynth_api.js';
var jsdoc_dest = output_dir + '/api';
var jsdoc_template = false;//'templates/haruki';

var os = require('os');
var path = require('path');
var fs = require('fs');
var marked = require('marked');
var child_process = require('child_process');

// Normalize paths from above
doc_dir = path.normalize(doc_dir);
output_dir = path.normalize(output_dir);
jsdoc_api = path.normalize(jsdoc_api);
jsdoc_dest = path.normalize(jsdoc_dest);

function mkdirs(dir) {
	console.log('mkdir: %s', dir);
	if (!fs.existsSync(dir)) {
		var parent = path.dirname(dir);
		if (!parent || parent == dir) {
			throw new Error("Bad path");
		}
		mkdirs(parent);
		// Now that our parents exist, we can create ourself.
		fs.mkdirSync(dir);
	}
}

/**
 * From http://stackoverflow.com/questions/11293857/fastest-way-to-copy-file-in-node-js
 */
function copy(source, dest, callback) {
	console.log("Copy: %s => %s", source, dest);
	var cbCalled = false;
	var rd = fs.createReadStream(source);
	rd.on("error", function(err) { done(err); });
	var wr = fs.createWriteStream(dest);
	wr.on("error", function(err) { done(err); });
	wr.on("close", function(ex) { done(); });
	rd.pipe(wr);
	function done(err) {
		if (!cbCalled) {
			callback(err);
			cbCalled = true;
		}
	}
}

mkdirs(output_dir);

// Our very simple template for the header and footer of the file.
var template = {
	head: "<!DOCTYPE html>\n\n<html>\n<head>\n<title>$TITLE</title>\n</head>\n<body>\n",
	foot: "</body>\n</html>\n"
};

var lexer = new marked.Lexer();

/**
 * Wrapper around actual marked used to do some "special" things to the
 * generated HTML.
 */
function jsvmarked(text) {
	var tokens = lexer.lex(text);
	var html = [];
	tokens.forEach(function(tok) {
		// Basically, check to see if there's a link in here...
		if ('text' in tok) {
			tok.text = tok.text.replace(/(\[.+?])\((.+?)\)/g, function(m, link, href) {
				// Check to see if the href is a relative link to an MD file
				if (href.length > 3 && href.charAt(0) != '/' && href.indexOf(':') < 0) {
					if (href.substring(href.length-3).toLowerCase() == '.md') {
						// If it is, make it an HTML link
						href = href.substring(0, href.length-3) + ".html";
					}
				}
				return link + "(" + href + ")";
			});
		}
	});
	return marked.parser(tokens);
}

// Pull in the contents of the doc dir

function createMarkdown(next) {
	var files = fs.readdirSync(doc_dir);
	
	files.forEach(function(file) {
		if (file.length > 3 && file.substring(file.length-3).toLowerCase() == '.md') {
			var htmlFile = path.join(output_dir, file.substring(0, file.length-3) + ".html");
			var mdFile = path.join(doc_dir, file);
			console.log("Marked: %s => %s", mdFile, htmlFile);
			var markdownText = fs.readFileSync(mdFile, {encoding:"utf8"});
			var html = jsvmarked(markdownText);
			var title = /<h1>(.*?)<\/h1>/.exec(html);
			if (title) {
				title = title[1];
			} else {
				title = file;
			}
			var head = template.head.replace(/\$TITLE/, title);
			var foot = template.foot;
			fs.writeFileSync(htmlFile, head + html + foot);
		}
	});
	next();
}

function copyCSS(next) {
	if (typeof css == 'string') {
		css = [ css ];
	}
	var copyFrom = new Array(css.length);
	var copyTo = new Array(css.length);
	css.forEach(function(file, idx) {
		// Step 1: normalize
		file = path.normalize(file);
		// Step 2: translate to "to" and "from" dirs
		var from = path.join(doc_dir, file);
		var to = path.join(output_dir, file);
		// Step 3: Make sure the to directory exists
		mkdirs(path.dirname(to));
		copyFrom[idx] = from;
		copyTo[idx] = to;
		console.log('Schedule[%d]: %s => %s', idx, from, to);
	});
	function copyNext(idx) {
		if (idx < copyFrom.length) {
			copy(copyFrom[idx], copyTo[idx], function(err) {
				if (err) {
					console.error("CSS copy failed!");
					console.error(err);
				} else {
					copyNext(++idx);
				}
			});
		} else {
			next();
		}
	}
	copyNext(0);
}

function runJSDoc(next) {
	// First see if we can even run JSDoc
	console.log("jsdoc: check if available");

	var jsdoc_command = 'jsdoc';
	var jsdoc_args = [ ];
	if (os.type() == 'Windows_NT') {
		// Because the JSDoc wrapper is a CMD file and CMD files are "special"
		// under Windows and not "normal processes", we have to instead invoke
		// CMD to run it.
		jsdoc_command = 'CMD';
		jsdoc_args = [ '/C', 'jsdoc' ];
	}

	var child = child_process.spawn(jsdoc_command, jsdoc_args.concat([ '--version' ]));
	child.stdout.pipe(process.stdout);
	child.stderr.pipe(process.stderr);
	child.on('error', function(err) {
		if (err.code == 'ENOENT') {
			console.log("JSDoc could not be found. In order to build the " +
				"JavaScript API documentation, please install JSDoc and place " +
				"it someplace on the path.\n\n" +
				"You can download JSDoc from: https://github.com/jsdoc3/jsdoc");
			console.error("jsdoc: skipping! (cannot be found)");
			next();
		} else {
			console.error("Unable to launch JSDoc (internal error)");
			console.error(err);
		}
	});
	child.on('exit', function(code) {
		if (code == 0) {
			// Success, we can run it
			console.log("jsdoc: %s => %s", jsdoc_api, jsdoc_dest);
			var args = jsdoc_args.concat([ jsdoc_api, '-d', jsdoc_dest ]);
			if (jsdoc_template) {
				args.push('-t', path.normalize(jsdoc_template));
			}
			var jsdoc = child_process.spawn(jsdoc_command, args);
			jsdoc.stdout.pipe(process.stdout);
			jsdoc.stderr.pipe(process.stderr);
			jsdoc.on('error', function(err) {
				console.error("Unable to launch JSDoc (internal error)");
				console.error(err);
			});
			jsdoc.on('exit', function(code) {
				if (code == 0) {
					next();
				} else {
					console.error("jsdoc finished with error (%s)", code);
				}
			});
		} else {
			console.log("jsdoc: Not available (see any errors above).");
			next();
		}
	});
}

// This exists mostly to make sure everything calls next properly
function finished() {
	console.log("Completed successfully.");
}

function run() {
	var chain = [ createMarkdown, copyCSS, runJSDoc, finished ];
	var i = 0;
	var execNext = function() {
		if (i < chain.length) {
			var now = chain[i];
			i++;
			now(execNext);
		}
	}
	execNext();
}

run();

