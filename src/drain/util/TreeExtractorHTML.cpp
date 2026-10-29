/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */

#include "Output.h"
#include "StringMapper.h"
#include "TreeElemUtilsHTML.h"

#include "TreeExtractorHTML.h"
// #include <drain/util/TreeXML.h>
// #include <drain/image/FilePng.h>
// #include <drain/image/TreeUtilsSVG.h>

namespace drain {


TreeHTML & TreeExtractorHTML::addTogglerScript(TreeHTML & head, const std::string key){

	/// Original code from: https://www.w3schools.com/howto/howto_js_treeview.asp

	static const StringBuilder<'\n'> builder = {"",
			"/* Original example: https://www.w3schools.com/howto/howto_js_treeview.asp */",
			"function addTogglers(){"
			"  var toggler = document.getElementsByClassName('caret');",
			"  for (var i = 0; i < toggler.length; i++) {",
			"     toggler[i].addEventListener('click', function() {",
			"        this.parentElement.querySelector('.nested').classList.toggle('active');",
			"        this.classList.toggle('caret-down');",
			"     });",
			"  }",
			"}",
	};

	TreeHTML & script = head[key](Html::SCRIPT);
	script->set("type", "text/javascript");
	script = builder.c_str();

	return script;
}

TreeHTML & TreeExtractorHTML::addTogglerStyle(TreeHTML & head, const std::string key){

	/// Original code from: https://www.w3schools.com/howto/howto_js_treeview.asp
	TreeHTML & style = head[key](Html::STYLE);
	style[Html::COMMENT](Html::COMMENT) = "Original example: https://www.w3schools.com/howto/howto_js_treeview.asp";
	style["ul, #myUL"] = "list-style-type: none;";
	style["#myUL"]->set("margin:0; padding:0;");
	style[".caret"]->set("cursor:pointer; user-select:none;");
	style[".caret::before"]->set("content:\"\\25B6\"; color:black; display:inline-block; margin-right:6px;");
	style[".caret-down::before"]->set("transform: rotate(90deg);");

	style[".nested"]->set("display: none;");
	style[".active"]->set("display: block;");
	/*
	style[".nested"] = {
			{"display", "none"}
	};
	style[".active"] = {
			{"display", "block"}
	};
	*/

	return style;
}

TreeHTML & TreeExtractorHTML::getHtml(){

	Logger submout(__FILE__, __FUNCTION__);

	if (html->isUndefined()){

		// submout.ok("initializing HTML");
		html(Html::HTML);

		// submout.ok("adding HEAD");
		TreeHTML & head = html[Html::HEAD](Html::HEAD); //  << NodeHTML::entry<NodeHTML::HEAD>();

		// submout.ok("adding TITLE");
		head[Html::TITLE](Html::TITLE) = "HDF5 file";

		/*
		TreeHTML & styleLink = head["styleLink"](Html::LINK);
		styleLink->set("rel",  "stylesheet");
		styleLink->set("href", "toggle.css");
		*/

		TreeHTML & style = head[Html::STYLE](Html::STYLE);
		style[Html::COMMENT](Html::COMMENT)->setText("Rack default style");

		//style["table,tr"] = "border:1px solid;";
		style["h1,h2,h3"]  = "font-family: Helvetica, sans-serif;";
		style["table,img"] = "border:1px solid;";
		style["table"] = "border-collapse:collapse; font-size:small; font-family:monospace";
		style["th"] = "text-align:left; font-decoration:none";
		style["tr:nth-child(even)"] = "background-color: #f0f0f0;";
		style[".where"] = "stroke-color:#4080f0";
		style[".what"]  = "stroke-color:#40f0a0";
		style[".how"]   = "stroke:blue; opacity:0.5";
		style[".metadata"] = "stroke-color:#40f0a0; font-size:small; font-family:monospace";

		addTogglerStyle(head);
		addTogglerScript(head);
		// TreeHTML & script = head["script2"](Html::SCRIPT);
		// script->set("type", "text/javascript");
		// script->set("src", "toggle.js");


		html[Html::BODY](Html::BODY);
		html[Html::BODY]->set("onload", "addTogglers()");

		submout.accept(html);

	}

	return html;

};


} // namespace drain
