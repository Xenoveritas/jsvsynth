#!/usr/bin/env node

// Quicky script to run marked over the Markdown documentation in the doc
// directory

var path = require('path');
var fs = require('fs');
var marked = require('marked');

var doc_dir = 'doc';
var output_dir = 'doc';

// Our very simple template for the header and footer of the file.
var template = {
	head: "<!DOCTYPE html>\n\n<html>\n<head>\n<title>$TITLE</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"jsvsynth.css\">\n</head>\n<body>\n",
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

var files = fs.readdirSync(doc_dir);

files.forEach(function(file) {
	if (file.length > 3 && file.substring(file.length-3).toLowerCase() == '.md') {
		var htmlFile = path.join(output_dir, file.substring(0, file.length-3) + ".html");
		var mdFile = path.join(doc_dir, file);
		console.log("Process: " + mdFile + " => " + htmlFile);
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
