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

#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeXML.h>
#include <drain/util/TreeHTML.h>

#include <drain/image/FilePng.h>
#include <drain/image/TreeUtilsSVG.h>

#include "data/SourceODIM.h" // for NOD
#include "fileio-html.h"  // ImageSection

namespace rack {





drain::TreeHTML & H5HTMLextractor::addTogglerScript(drain::TreeHTML & head, const std::string key){

	/// Original code from: https://www.w3schools.com/howto/howto_js_treeview.asp

	static const drain::StringBuilder<'\n'> builder = {"",
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

	drain::TreeHTML & script = head[key](drain::BaseHTML::SCRIPT);
	script->set("type", "text/javascript");
	script = builder.c_str();

	return script;
}

drain::TreeHTML & H5HTMLextractor::addTogglerStyle(drain::TreeHTML & head, const std::string key){

	/// Original code from: https://www.w3schools.com/howto/howto_js_treeview.asp
	drain::TreeHTML & style = head[key](drain::BaseHTML::STYLE);
	// script->set("type", "text/javascript");
	style["comment"](drain::BaseHTML::STYLE)->setText("/* Original example: https://www.w3schools.com/howto/howto_js_treeview.asp */");
	style["ul, #myUL"] = "list-style-type: none;";
	style["#myUL"]->set("margin:0; padding:0;");
	style[".caret"]->set("cursor:pointer; user-select:none;");
	style[".caret::before"]->set("content:\"\\25B6\"; color:black; display:inline-block; margin-right:6px;");
	style[".caret-down::before"]->set("transform: rotate(90deg);");
	style[".nested"]->set("display: none;");
	style[".active"]->set("display: block;");

	return style;
}

drain::TreeHTML & H5HTMLextractor::getHtml(){

	drain::Logger submout(__FILE__, __FUNCTION__);

	if (html->isUndefined()){

		// submout.ok("initializing HTML");
		html(drain::BaseHTML::HTML);

		// submout.ok("adding HEAD");
		drain::TreeHTML & head = html["head"](drain::BaseHTML::HEAD); //  << drain::NodeHTML::entry<drain::NodeHTML::HEAD>();

		// submout.ok("adding TITLE");
		head["title"](drain::BaseHTML::TITLE) = "HDF5 file";

		/*
		drain::TreeHTML & styleLink = head["styleLink"](drain::BaseHTML::LINK);
		styleLink->set("rel",  "stylesheet");
		styleLink->set("href", "toggle.css");
		*/

		drain::TreeHTML & style = head["style"](drain::BaseHTML::STYLE);
		//style["table,tr"] = "border:1px solid;";
		style["h1,h2,h3"] = "font-family: Helvetica, sans-serif;";
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
		// drain::TreeHTML & script = head["script2"](drain::BaseHTML::SCRIPT);
		// script->set("type", "text/javascript");
		// script->set("src", "toggle.js");


		html["body"](drain::BaseHTML::BODY);
		html["body"]->set("onload", "addTogglers()");

		submout.accept(html);

	}

	return html;

};


int H5HTMLextractor::visitPrefix(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath){

	drain::Logger submout(__FILE__, __FUNCTION__);

	// Yes.
	const Hi5Tree & t = tree(odimPath);

	// std::cout << path << ':'  << '\n'; // << tree(path).data

	if (t.data.exclude){
		// submout.warn("excluding ", odimPath);
		return 1;
	}

	submout.special<LOG_DEBUG>("visiting ", odimPath);

	drain::TreeHTML & htmlDoc = getHtml();

	drain::TreeHTML & body = htmlDoc["body"];
	body["ul"](drain::BaseHTML::UL); // prevent nested class

	drain::TreeHTML::path_t htmlPath;

	// Expand the path to html path UL->LI->UL-> by adding an element (LI) after each
	for (const ODIMPathElem & e: odimPath){

		// submout.special(" elem ", e);

		const std::string & elemName = e.str();

		htmlPath.appendElem("ul");
		// submout.ok("checking path: ", htmlPath);

		drain::TreeHTML & group = body(htmlPath); //(drain::BaseHTML::UL);
		if (group->isUndefined()){
			group->setType(drain::BaseHTML::UL);
			group->addClass("nested");
		}

		if (e.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){  // what, where or how
			// Attrib groups joins are joined together (instead of creating a separate table for each
			drain::TreeHTML & table = group["attr"](drain::NodeHTML::TABLE);
			for (const auto & attr: t.data.attributes){
				drain::TreeHTML & tr = table[elemName + attr.first](drain::NodeHTML::TR);
				tr->addClass(elemName); // what, where or how
				tr["key"](drain::NodeHTML::TH) = elemName+':'+attr.first;
				tr["value"](drain::NodeHTML::TD) = attr.second;
			}
			return 1; // = do  not traverse subtrees
		}
		else {
			htmlPath.appendElem(elemName);
			drain::TreeHTML & item = body(htmlPath); // (drain::NodeHTML::LI);
			if (item->isUndefined()){
				item->setType(drain::BaseHTML::LI);
				// item->set("name", estr);
				if (e.is(ODIMPathElem::ARRAY)){

					item->addClass("array");

					drain::TreeHTML & img = item["img"](drain::NodeHTML::IMG);
					img->set("title", odimPath.str());

					const drain::image::Image & image = t.data.image;

					drain::StringBuilder<> builder(odimPath.str(),'-',image.properties.get("what:quantity","unknown"),".png");
					const std::string & filename = builder;
					// img->set("alt", filename);

					drain::FilePath relativePath(html->getId(), filename);
					submout.debug("relative path:", relativePath);
					img->set("src", relativePath.str());

					drain::FilePath fullPath(basedir, relativePath);
					submout.debug("full path:", fullPath);

					try {
						if (!fullPath.dir.empty()){
							submout.debug("ensuring dir: ", fullPath.dir);
							drain::FilePath::mkdir(fullPath.dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
						}
						drain::image::FilePng::write(image, fullPath.str());
					}
					catch (const std::exception & e) {
						submout.warn("error(s): ", e.what());
						submout.warn("could not write file: ", fullPath);
					}

				}
				else {
					drain::TreeHTML & span = body(htmlPath)[elemName+"-span"]; // (drain::BaseHTML::SPAN);
					if (span->isUndefined()){
						span->setType(drain::BaseHTML::SPAN);
						span->addClass("caret");
						span = elemName+'/';
					}
					// Mark elevation (elangle) or quantity
					if (e.is(ODIMPathElem::DATASET)){
						const drain::VariableMap & v = t[ODIMPathElem::WHERE].data.attributes;
						if (v.hasKey("elangle")){ // body(htmlPath)
							drain::TreeHTML & info = span[elemName+"-elangle"](drain::BaseHTML::SPAN);
							info->addClass("metadata");
							info = v["elangle"];
						}
					}
					else if (e.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
						const drain::VariableMap & v = t[ODIMPathElem::WHAT].data.attributes;
						//if (v.hasKey("quantity")){ body(htmlPath)
						drain::TreeHTML & info = span[elemName+"-elangle"](drain::BaseHTML::SPAN);
						info->addClass("metadata");
						info = v.get("quantity", "unknown");
					}
				}

			}
			/*
			else {
				bool EXISTS = body.hasPath(htmlPath);
				submout.attention("LI group existed=", (EXISTS?"yes":"no"), ", ", item.data, " at ", htmlPath);
			}
			*/
		}


	}


	return 0;
}



} // namespace rack
