/**
 * This is a custom jsdoc template for JSVSynth, designed to make
 * the JavaScript API docs look less terrible. It also allows some
 * custom tags like @future marking unimplemented APIs.
 */

var fs = require('fs');
var path = require('path');
var _ = require('underscore');

function escapeHTML(str) {
	return String(str).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

function createWriteStream(path, options) {
	return new Writer(path, options);
}

function Writer(filename, opts) {
	this.filename = filename;
	this.opts = opts;
	this.data = [];
}

Writer.prototype = {
	write: function(data) {
		this.data.push(data.toString());
	},
	end: function(data) {
		if (data) {
			this.data.push(data.toString());
		}
		fs.writeFileSync(this.filename, this.data.join(''), this.opts.encoding);
	}
};

function JSVSynthTemplate(data, destination, encoding, options, readme, templatePath) {
	this.data = data;
	this.destination = destination;
	this.encoding = encoding;
	this.bootstrap = escapeHTML(options['bootstrap']);
	this.title = options['title'];
	this.readme = readme;
	var bootstrap = this.bootstrap;
	if (bootstrap.length > 0 && (bootstrap.charAt(bootstrap.length-1) != '/'))
		bootstrap = bootstrap + '/';
	this.css_links = this.stylesheets.reduce(function(left, right) {
		return left + '<link rel="stylesheet" type="text/css" href="' + bootstrap + right + '">\n';
	}, "");
	this.stylesheets = this.stylesheets.map(function(sheet) {
		return bootstrap + sheet;
	});
	this.pageTemplate = _.template(fs.readFileSync(path.join(templatePath, "template.html"), "utf-8"));
	this.indexPageTemplate = _.template(fs.readFileSync(path.join(templatePath, "index.tmpl"), "utf-8"));
}

JSVSynthTemplate.prototype = {
	stylesheets: [ 'css/bootstrap.min.css', 'css/bootstrap-theme.min.css' ],
	/**
	 * Set of names that are "reserved" and therefore can't have pages named after them.
	 */
	reserved: { "index": true, "globals": true, "classes": true, "functions": true },
	/**
	 * Kinds of things and their names.
	 */
	kindNames: {
		"class": "Classes",
		"namespace": "Namespaces",
		"mixin": "Mixins",
		"global": "Globals",
		"member": "Members",
		"function": "Functions"
	},
	/**
	 * A list of things to show on the index page.
	 */
	indexPageObjects: [ "namespace", "class", "mixin", "global" ],
	/**
	 * The order to show things on a single thing page.
	 */
	thingPageObjects: [ "namespace", "class", "mixin", "member", "function" ],
	writeHeader: function(out, title) {
		out.write('<!DOCTYPE html>\n\
\n\
<html>\n\
  <head>\n\
	<meta charset="UTF-8">\n\
	<title>');
		out.write(escapeHTML(title));
		out.write('</title>\n');
		var bootstrap = this.bootstrap;
		this.stylesheets.forEach(function(sheet) {
			out.write('	<link rel="stylesheet" type="text/css" href="');
			out.write(bootstrap + '/' + sheet);
			out.write('">\n');
		});
		out.write('\t<style type="text/css">body { margin-top: 60px }</style>\n  </head>\n\  <body>\n');
	},
	writeNavBar: function(out) {
		out.write('	<div class="navbar navbar-inverse navbar-fixed-top">\n\
		<div class="container">\n\
			<div class="navbar-header">\n\
				<button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">\n\
					<span class="icon-bar"></span>\n\
					<span class="icon-bar"></span>\n\
					<span class="icon-bar"></span>\n\
				</button>\n\
				<a class="navbar-brand" href="#">');
		out.write(escapeHTML(this.title));
		out.write('</a>\n\
			</div>\n\
			<div class="collapse navbar-collapse">\n\
				<ul class="nav navbar-nav">\n\
					<li class="active"><a href="index.html">Index</a></li>\n\
					<li><a href="#">Classes</a></li>\n\
					<li><a href="#">Functions</a></li>\n\
				</ul>\n\
			</div>\n\
		</div>\n\
	</div>\n');
	},
	writePanelHeader: function(out, title, type) {
		if (!type) {
			type = "panel-default";
		}
		out.write('<div class="panel ' + type + '"><div class="panel-heading">' + title + '</div><div class="panel-body">');
	},
	writePanelFooter: function(out) {
		out.write('</div></div>');
	},
	/**
	 * Write a navigation panel based on an array of links. Each entry in the array
	 * should be an object containing an href and a text field.
	 */
	writeNavPanel: function(out, title, links) {
		if (links.length == 0)
			return;
		out.write('<div class="panel panel-default"><div class="panel-heading">');
		out.write(title);
		out.write('</div><div class="panel-body"><ul class="nav nav-pills nav-stacked">');
		var me = this;
		links.forEach(function(link) {
			out.write('<li>');
			if ('href' in link) {
				out.write('<a href="' + link['href'] + '">');
				if ('text' in link) {
					out.write(link['text']);
				} else {
					out.write(escapeHTML(link['href']));
				}
			} else if ('type' in link) {
				out.write(me.makeLink(link['type'], link['text']));
			}
			out.write('</li>');
		});
		out.write('</ul></div></div>');
	},
	createIndex: function() {
		// Needed for closures:
		var me = this;
		var readme = false;
		// If we have a readme, create it.
		if (this.readme) {
			readme = this.replaceLinks(this.readme);
		}
		fs.writeFileSync(path.join(this.destination, "index.html"),
			this.pageTemplate(
				{
					"title": this.title,
					"stylesheets": this.stylesheets,
					"body": this.indexPageTemplate({
						"readme": readme,
						"categories": []
					})
				}),
			"utf-8");
		return;
		// Start by creating the body HTML.
		var body = [];
		// Add the README section.
		out.write('\t<div class="container">\n\t\t<div class="row">\n\t\t\t<div class="col-md-12">');
		if (this.readme) {
			out.write(this.replaceLinks(this.readme));
		}
		out.write('</div>\n\t\t</div>\n\t\t<div class="row">\n\t\t\t<div class="col-md-12">');
		this.indexPageObjects.forEach(function(kind) {
			var things = me.getAllOfKind(kind);
			var links = new Array(things.length);
			things.forEach(function(thing) {
				links.push({'type':thing.longname,'text':thing.name});
			});
			me.writeNavPanel(out, me.lookupKindName(kind), links);
		});
		out.write('\n\
			</div>\n\
		</div>\n\
	</div>\n');
		out.end('  </body>\n</html>\n');
	},
	lookupKindName: function(kind) {
		if (kind in this.kindNames) {
			return this.kindNames[kind];
		} else {
			return kind;
		}
	},
	getAllOfKind: function(kind) {
		if (kind == 'global') {
			// this is a special instance
			return this.data({'scope':'global'}).order("name").get();
		} else {
			return this.data({'kind':kind}).order("name").get();
		}
	},
	createPage: function(cls) {
		var out = createWriteStream(this.makePath(cls.longname), {'encoding': this.encoding});
		this.writeHeader(out, this.title + ": " + cls.longname);
		this.writeNavBar(out);
		out.write('\t<div class="container">\n\t\t<div class="row">\n\t\t\t<div class="col-md-12">\n\t\t\t\t');
		out.write(this.replaceLinks(cls.description));
		out.write('\n\t\t\t</div>\n\t\t</div>\n');
		// Now that we've got the description, dump out a list of other junk
		var me = this;
		this.thingPageObjects.forEach(function(kind) {
			var things = me.data({'memberof':cls.longname,'kind':kind}).order('name').get();
			if (things.length > 0) {
				out.write('<div class="row"><div class="col-md-12"><h1>');
				out.write(me.lookupKindName(kind));
				out.write('</h1>\n');
				things.forEach(function(thing) {
					me.writeThing(out, thing);
				});
				out.write('</div></div>\n');
			}
		});
		var things = this.data({'memberof':cls.longname}).get();
		things.forEach(function(thing) {
			out.write("&lt;" + thing.kind + "&gt; " + thing.longname + '<br>');
		});
		out.end('\t</div>\n  </body>\n</html>\n');
	},
	createBadges: function(thing) {
		var rv = [];
		if (thing.readonly) {
			rv.push('<span class="badge">read only</span>');
		}
		return rv.length == 0 ? '' : ' ' + rv.join(' ');
	},
	createTypes: function(type) {
		if (!type) {
			return '<div class="type type-none">no type specified</div>';
		}
		if (type == 'NUMBER') {
			return '<div class="type type-number">number</div>';
		}
		if (type.names) {
			var rv = [];
			type.names.forEach(function(t) {
				rv.push(escapeHTML(String(t)));
			});
			return '<div class="type">' + escapeHTML(rv.join(', ')) + '</div>';
		} else {
			return '<div class="type">' + escapeHTML(String(type)) + '</div>';
		}
	},
	writeThing: function(out, thing) {
		switch(thing.kind) {
		case 'member':
			this.writeMember(out, thing);
			break;
		case 'function':
			this.writeFunction(out, thing);
			break;
		default:
			out.write('<p>No writer for type ' + escapeHTML(thing.kind) + '</p>');
		}
	},
	writeMember: function(out, member) {
		this.writePanelHeader(out, '<a name="' + member.name + '">' + escapeHTML(member.name) + this.createBadges(member) + '</a>');
		out.write(this.createTypes(member.type));
		out.write(this.replaceLinks(member.description));
		//out.write('<pre>' + escapeHTML(JSON.stringify(member, null, 2)) + '</pre>');
		console.log(member);
		this.writePanelFooter(out);
	},
	writeFunction: function(out, func) {
		this.writePanelHeader(out, '<a name="' + func.name + '">' + escapeHTML(func.name) + this.createBadges(func) + '</a>');
		out.write(this.replaceLinks(func.description));
		this.writePanelFooter(out);
		console.log(func);
	},
	publish: function() {
		this.createIndex();
		// Create pages for each class...
		var me = this;
		this.data({"kind":["class","namespace"]}).get().forEach(function(cls) {
			me.createPage(cls);
		});
	},
	replaceLinks: function(text) {
		text = String(text);
		var me = this;
		return text.replace(/{@link(code)?\s+([^\s}]+)(?:\s+([^}]+))?\s*}/g, function(match, type, linkTo, linkText) {
			if (!linkText) {
				linkText = escapeHTML(linkTo);
				// Because we always escape the linkText for HTML codes in this case, we need
				// to wrap with <code> here
				if (type == 'code') {
					linkText = '<code>' + linkText + '</code>';
				}
			} else if (type == 'code') {
				linkText = "<code>" + escapeHTML(linkText) + "</code>";
			}
			return me.makeLink(linkTo, linkText);
		});
	},
	getNamedObject: function(name) {
		return this.data({"name":name});
	},
	/**
	 * Transform something into a link.
	 * @param to   the thing being linked to
	 * @param text the text to use for linking the text, HTML is <em>not</em> escaped
	 */
	makeLink: function(to, text) {
		if (!text)
			text = to;
		// FIXME: Should look up the type and then make the link only if it exists
		return '<a href="' + this.makeURL(to) + '">' + text + '</a>';
	},
	makeURL: function(toType) {
		// FIXME: Should look up the type and then make the link depending on the type
		// (functions and members should link to the parent type, not the page)
		return this.makeFileName(toType);
	},
	makeFileName: function(toType) {
		var name = String(toType);
		// Replace "reserved" characters
		name = name.replace(/_/g, '__').replace(/\$/g, '_.').replace(/([^\w_\. ])/g, function(m,c) {
			var r = c.charCodeAt(0).toString(16);
			return "0000".substring(r.length) + r;
		});
		if (name in this.reserved) {
			name = name + "_";
		}
		return name + '.html';
	},
	makePath: function(toType) {
		return path.join(this.destination, this.makeFileName(toType));
	}
};

exports.publish = function(data, opts) {
	console.log(opts);
	var configuration = {
		'bootstrap': 'bootstrap',
		'title': 'JavaScript'
	};
	if ('configure' in opts) {
		var conf = fs.readFileSync(opts['configure'], 'utf8');
		conf = JSON.parse(conf);
		// Copy everything over
		for (var k in conf) {
			configuration[k] = conf[k];
		}
	}
	var dest = 'jsdoc';
	if ('destination' in opts) {
		dest = opts['destination'];
	}
	var encoding = 'utf8';
	if ('encoding' in opts) {
		encoding = opts['encoding'];
	}
	var template = new JSVSynthTemplate(data, dest, encoding, configuration, opts['readme'], opts['template']);
	template.publish();
}