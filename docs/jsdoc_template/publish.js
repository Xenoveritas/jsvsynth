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

function copyFile(from, to) {
	// For now:
	var contents = fs.readFileSync(from);
	fs.writeFileSync(to, contents);
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
	this.templatePath = templatePath;
	var bootstrap = this.bootstrap;
	if (bootstrap.length > 0 && (bootstrap.charAt(bootstrap.length-1) != '/'))
		bootstrap = bootstrap + '/';
	var bsSheets = this.bootstrapStylesheets.map(function(sheet) {
		return bootstrap + sheet;
	});
	var bsScripts = this.bootstrapScripts.map(function(script) {
		return bootstrap + script;
	});
	this.stylesheets = bsSheets.concat(this.stylesheets);
	this.scripts = bsScripts;
	this.pageTemplate = _.template(fs.readFileSync(path.join(templatePath, "template.html"), "utf-8"));
	this.indexPageTemplate = _.template(fs.readFileSync(path.join(templatePath, "index.tmpl"), "utf-8"));
	this.classPageTemplates = this._loadTemplates(path.join(templatePath, "class.tmpl"));
	this.classPageTemplate = this.classPageTemplates['body'];
	// Pull out the kind templates into their own object
	this.kindTemplates = {};
	for (var k in this.classPageTemplates) {
		if (k.substring(0,5) == 'kind.')
			this.kindTemplates[k.substring(5)] = this.classPageTemplates[k];
	}
	// Now that everything is configured, embed links into each of the things in the data array
	var me = this;
	this.data().get().forEach(function(thing) {
		thing.url = me.makeURL(thing);
	});
}

JSVSynthTemplate.prototype = {
	bootstrapStylesheets: [ 'css/bootstrap.min.css', 'css/bootstrap-theme.min.css' ],
	bootstrapScripts: [ 'js/jquery-2.0.3.min.js', 'js/bootstrap.min.js' ],
	stylesheets: [ 'jsdoc.css' ],
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
	_loadTemplates: function(filename) {
		var template = fs.readFileSync(filename, "utf-8");
		// See if this template contains any sub-sections
		var sectionStart = 0, searchStart = 0;
		var currentSection = "__root__";
		var result = {};
		var includes = {};
		while (true) {
			var idx = template.indexOf("<!--#section", searchStart);
			if (idx < 0) {
				// We found the last template.
				var t = template.substring(sectionStart);
				result[currentSection] = this._createTemplate(t, includes);
				includes[currentSection] = t;
				break;
			} else {
				// See if we can find the name of this section
				var nIdx = template.indexOf("-->", idx);
				if (nIdx > 0) {
					// See if we can find a name in the section
					var m = /^\s*"(.*)"\s*$/.exec(template.substring(idx + 12, nIdx));
					if (m) {
						// We have a new match. Create the current template.
						var t = template.substring(sectionStart, idx);
						result[currentSection] = this._createTemplate(t, includes);
						includes[currentSection] = t;
						currentSection = m[1];
						sectionStart = searchStart = nIdx+3;
						continue;
					}
				}
				// If we've fallen through to here, we didn't find a valid section marker, so just
				// advance the search position.
				searchStart = idx + 12;
			}
		}
		/*for (var k in result) {
			console.log(k + '=' + result[k].source);
		}*/
		return result;
	},
	_createTemplate: function(template, includes) {
		try {
			if (arguments.length > 1) {
				// Slot in any includes.
				template = template.replace(/<!--#\s*include\s+"([^"]+)"\s*-->/g, function(m, name) {
					return name in includes ? includes[name] : "[an error occurred while processing this directive]";
				});
			}
			return _.template(template);
		} catch (ex) {
			console.log("Unable to create template:");
			console.log(template);
			console.log(ex.toString());
			throw ex;
		}
	},
	/**
	 * Write a page, embedding it in the standard page template.
	 * @param {string} filename the filename to use
	 * @param {string} title the page title
	 * @param {string} body the body HTML
	 */
	writePage: function(filename, title, body, navbar) {
		fs.writeFileSync(path.join(this.destination, filename),
			this.pageTemplate({
				"title": title,
				"rootTitle": this.title,
				"rootURL": "index.html",
				"stylesheets": this.stylesheets,
				"scripts": this.scripts,
				"body": body,
				"navbar": navbar
			}), "utf-8");
	},
	createIndex: function() {
		// Needed for closures:
		var me = this;
		var readme = false;
		// If we have a readme, create it.
		if (this.readme) {
			readme = this.replaceLinks(this.readme);
		}
		var categories = [];
		var navbar = [];
		// Go through each thing that can be displayed on the index page and add them if they exist.
		this.indexPageObjects.forEach(function(kind) {
			categories.push({
				"contents": me.getAllOfKind(kind),
				"title": me.lookupKindName(kind)
			});
			navbar.push({
				'text': me.lookupKindName(kind),
				'url': '#' + kind
			});
		});
		this.writePage("index.html", this.title, this.indexPageTemplate({
				"title": this.title,
				"readme": readme,
				"categories": categories
			}), navbar);
		return;
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
		var me = this;
		var description = this.replaceLinks(cls.description);
		var title = this.title + ": " + cls.longname;
		var categories = new Array(this.thingPageObjects.length);
		var navbar = [];
		this.thingPageObjects.forEach(function(kind) {
			var things = me.data({'memberof':cls.longname,'kind':kind}).get();
			categories.push({
				"contents": things,
				"title": me.lookupKindName(kind),
				"kind": kind,
				"kindTemplate": (kind in me.kindTemplates) ? me.kindTemplates[kind] : null
			});
			var navchildren = new Array(things.length);
			things.forEach(function(thing) {
				navchildren.push({
					'text': thing.name,
					'url': '#' + thing.name
				});
			});
			navbar.push({
				'text': me.lookupKindName(kind),
				'url': '#.' + kind,
				'children': navchildren
			});
		});
		this.writePage(this.makeFileName(cls.longname), title, this.classPageTemplate({
			"thing": cls,
			"description": description,
			"categories": categories,
			"template": me,
			"templates": me.classPageTemplates
		}), navbar);
	},
	createGlobals: function() {
		// This very nearly is the same as createPage, but...
		var me = this;
		// There is no description (should this reuse the readme?)
		var description = '';
		var title = this.title + ": Globals";
		var categories = new Array(this.thingPageObjects.length);
		this.thingPageObjects.forEach(function(kind) {
			// Look up things in the global scope
			var things = me.data({'scope':'global','kind':kind}).get();
			categories.push({
				"contents": things,
				"title": me.lookupKindName(kind),
				"kind": kind,
				"kindTemplate": (kind in me.kindTemplates) ? me.kindTemplates[kind] : null
			});
		});
		this.writePage("globals.html", title, this.classPageTemplate({
			"thing": { "longname": "Globals", "name": "Globals" },
			"description": description,
			"categories": categories,
			"template": me,
			"templates": me.classPageTemplates
		}));
	},
	publish: function() {
		// First things first, create the destination if it does not exist.
		if (!fs.existsSync(this.destination)) {
			console.log("make " + this.destination);
			if (fs.mkPath) {
				fs.mkPath(this.destination);
			} else {
				fs.mkdirSync(this.destination);
			}
		}
		// For now, always copy our CSS
		copyFile(path.join(this.templatePath, 'jsdoc.css'), path.join(this.destination, 'jsdoc.css'));
		console.log("Creating index page...");
		this.createIndex();
		console.log("Creating globals page...");
		this.createGlobals();
		// Create pages for each class...
		var me = this;
		this.data({"kind":["class","namespace","mixin"]}).get().forEach(function(cls) {
			console.log("Creating " + cls.longname + " page...");
			me.createPage(cls);
		});
	},
	replaceLinks: function(text) {
		if (text == null || (typeof text) == 'undefined')
			return text;
		text = String(text);
		var me = this;
		return text.replace(/{@link(code)?\s+([^\s}]+)(?:\s+([^}]+))?\s*}/g, function(match, type, linkTo, linkText) {
			if (!linkText) {
				// Escape HTML and replace any '#' with '.'
				var linkedObj = me.getNamedObject(linkTo);
				linkText = escapeHTML(linkTo.replace(/#/g, '.'));
				if (linkedObj && (linkedObj.kind == 'function')) {
					linkText = linkText + '()';
				}
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
		var rv = this.data({"longname":name}).get();
		if (rv.length > 1) {
			console.log("Warning: Multiple answers for type \"" + name + "\"?");
		}
		return rv.length == 0 ? null : rv[0];
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
		if (typeof toType == 'string') {
			// Assume this is the name of a thing to look up.
			var to = this.data({'longname':toType}).get();
			if (to.length == 0) {
				// can't do anything.
				return "[bad link to " + toType + "]";
			}
			toType = to[0];
		}
		if (toType.scope == 'global' && toType.kind == 'member') {
			// Special
			return "globals.html#" + this.makeAnchor(toType.name);
		}
		if ((toType.kind == 'member' || toType.kind == 'function') && toType.memberof) {
			return this.makeFileName(toType.memberof) + '#' + this.makeAnchor(toType.name);
		}
		return this.makeFileName(toType.longname);
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
	makeAnchor: function(toName) {
		// FIXME: Escape whatever needs to be escaped
		return toName;
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