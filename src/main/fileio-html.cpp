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
#include <drain/util/TreeElemUtilsHTML.h>
#include <drain/util/TreeHTML.h>
//#include <drain/util/TreeXML.h>

#include <drain/image/FilePng.h>
// #include <drain/image/TreeUtilsSVG.h>

#include "data/SourceODIM.h" // for NOD
#include "fileio-html.h"  // ImageSection

namespace rack {





drain::TreeHTML & ExtractorH5toHTML::addTogglerScript(drain::TreeHTML & head, const std::string key){

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

	drain::TreeHTML & script = head[key](drain::Html::SCRIPT);
	script->set("type", "text/javascript");
	script = builder.c_str();

	return script;
}

drain::TreeHTML & ExtractorH5toHTML::addTogglerStyle(drain::TreeHTML & head, const std::string key){

	/// Original code from: https://www.w3schools.com/howto/howto_js_treeview.asp
	drain::TreeHTML & style = head[key](drain::Html::STYLE);
	style[drain::Html::COMMENT](drain::Html::COMMENT) = "Original example: https://www.w3schools.com/howto/howto_js_treeview.asp";
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

drain::TreeHTML & ExtractorH5toHTML::getHtml(){

	drain::Logger submout(__FILE__, __FUNCTION__);

	if (html->isUndefined()){

		// submout.ok("initializing HTML");
		html(drain::Html::HTML);

		// submout.ok("adding HEAD");
		drain::TreeHTML & head = html[drain::Html::HEAD](drain::Html::HEAD); //  << drain::NodeHTML::entry<drain::NodeHTML::HEAD>();

		// submout.ok("adding TITLE");
		head[drain::Html::TITLE](drain::Html::TITLE) = "HDF5 file";

		/*
		drain::TreeHTML & styleLink = head["styleLink"](drain::Html::LINK);
		styleLink->set("rel",  "stylesheet");
		styleLink->set("href", "toggle.css");
		*/

		drain::TreeHTML & style = head[drain::Html::STYLE](drain::Html::STYLE);
		style[drain::Html::COMMENT](drain::Html::COMMENT)->setText("Rack default style");

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
		// drain::TreeHTML & script = head["script2"](drain::Html::SCRIPT);
		// script->set("type", "text/javascript");
		// script->set("src", "toggle.js");


		html[drain::Html::BODY](drain::Html::BODY);
		html[drain::Html::BODY]->set("onload", "addTogglers()");

		submout.accept(html);

	}

	return html;

};


int ExtractorH5toHTML::visitPrefix(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath){

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

	drain::TreeHTML & body = htmlDoc[drain::Html::BODY];
	body[drain::Html::UL](drain::Html::UL); // prevent nested class

	drain::TreeHTML::path_t htmlPath;

	// static const std::string COMPLETED = "completed";

	// Expand the path to html path UL->LI->UL-> by adding an element (LI) after each
	for (const ODIMPathElem & e: odimPath){

		// submout.special(" elem ", e);
		const std::string & elemName = e.str();

		drain::TreeHTML & current = body(htmlPath); //(drain::Html::UL);

		if (!current.hasChild(drain::Html::UL)){
			current[drain::Html::UL](drain::Html::UL)->addClass("nested");
		}
		htmlPath.appendElem(drain::Html::UL);

		/* Tried this simple scheme but did not work...
		if (!current.hasChild(elemName)){
			current[elemName](drain::Html::UL)->addClass("nested");
		}
		htmlPath.appendElem(elemName);
		*/

		// submout.ok("checking path: ", htmlPath);

		drain::TreeHTML & group = body(htmlPath); //(drain::Html::UL);


		if (e.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){  // what, where or how

			// Attrib groups joins are joined together (instead of creating a separate table for each
			drain::TreeHTML & table = group[drain::NodeHTML::TABLE](drain::NodeHTML::TABLE); // "attr"
			for (const auto & attr: t.data.attributes){
				drain::TreeHTML & tr = table[attr.first]; // AUTO: (drain::NodeHTML::TR);
				tr->addClass(elemName); // what, where or how
				tr["key"](drain::NodeHTML::TH) = elemName+':'+attr.first;
				tr["value"](drain::NodeHTML::TD) = attr.second;
			}

			return 1; // = do  not traverse subtrees

		}
		else {


			if (group.hasChild(elemName)){
				submout.pending<LOG_DEBUG>("already exists: '", htmlPath, '|', elemName, "': ", group.data); //, '|', elemName
			}
			else {

				drain::TreeHTML & item = group[elemName](drain::Html::LI);

				submout.accept<LOG_INFO>("populating '", htmlPath, '|', elemName, "' of type ", group->getTag());

				if (e.is(ODIMPathElem::ARRAY)){

					item->addClass("array");

					drain::StringBuilder<> filepath(odimPath.str(),'-',t.data.image.properties.get("what:quantity","unknown"),".png");

					drain::FilePath relativePath(html->getId(), filepath.str()); // NOTE: str() needed, bug in dir detection...
					// submout.accept<LOG_WARNING>("relative path1:", relativePath);
					// drain::FilePath relativePath2(html->getId(), builder);
					// submout.reject<LOG_WARNING>("relative path2:", relativePath2);

					drain::NodeHTML::Elem<drain::Html::tag_t::IMG> imgElem(item[drain::NodeHTML::IMG]);
					imgElem.title = odimPath.str();
					imgElem.src = relativePath;

					drain::FilePath fullPath(basedir, relativePath);
					submout.debug("full path:", fullPath);

					try {
						if (!fullPath.dir.empty()){
							submout.debug("ensuring dir: ", fullPath.dir);
							drain::FilePath::mkdir(fullPath.dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
						}
						drain::image::FilePng::write(t.data.image, fullPath.str());
					}
					catch (const std::exception & e) {
						submout.warn("error(s): ", e.what());
						submout.warn("could not write file: ", fullPath);
					}

				}
				else {
					drain::TreeHTML & span = item[drain::Html::SPAN](drain::Html::SPAN); // (drain::Html::SPAN);
					span->addClass("caret");
					span->setText(elemName,'/');
					// Mark elevation angle or quantity
					if (e.is(ODIMPathElem::DATASET)){
						const drain::VariableMap & v = t[ODIMPathElem::WHERE].data.attributes;
						if (v.hasKey("elangle")){ // body(htmlPath)
							drain::TreeHTML & info = span["elangle"](drain::Html::SPAN); // elemName+
							info->addClass("metadata");
							info->setText(v["elangle"]);
						}
					}
					else if (e.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
						drain::TreeHTML & info = span["quantity"](drain::Html::SPAN); // elemName+"-elangle"
						info->addClass("metadata");
						const drain::VariableMap & v = t[ODIMPathElem::WHAT].data.attributes;
						info->setText(v.get("quantity", "unknown"));
					}
				}

			}
			/*
			else {
				bool EXISTS = body.hasPath(htmlPath);
				submout.attention("LI group existed=", (EXISTS?"yes":"no"), ", ", item.data, " at ", htmlPath);
			}
			*/

			htmlPath.appendElem(elemName);

		}


	}


	return 0;
}



} // namespace rack
