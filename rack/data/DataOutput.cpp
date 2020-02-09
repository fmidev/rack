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


#include "ODIMPath.h"
#include "DataOutput.h"

namespace rack {

//using namespace hi5;

std::string DataOutput::quoted(const ODIMPath & path){
	//std::stringstream sstr;
	if (path.empty() || path.back().isRoot())
		return "root";

	std::string str;
	//if (path.si)
	path.toStr(str, '_');
	return str;
}

void DataOutput::writeToDot(std::ostream & ostr, const Hi5Tree & tree, ODIMPathElem::group_t selector){

	drain::Logger mout("DataOutput", __FUNCTION__);

	int index = 0;

	/*
	//(ODIMPathElem::group_t selector = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED);
	ODIMPathElem::group_t selector = (ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

	const int verbosity = drain::getLog().getVerbosity();

	if (verbosity >= LOG_NOTICE){

		selector |= ODIMPathElem::WHAT;
		selector |= ODIMPathElem::WHERE;
		if (verbosity >= LOG_INFO){
			selector |= ODIMPathElem::ARRAY;
			if (verbosity >= LOG_DEBUG){
				selector |= ODIMPathElem::HOW;
				selector |= ODIMPathElem::OTHER;
			}
		}

	}
	 */

	ostr << "digraph G { \n";
	ostr << "/* selector=" << selector << " */  \n";

	DataOutput::writeGroupToDot(ostr, tree, index, selector);
	ostr << "}\n";

}

void DataOutput::writeGroupToDot(std::ostream & ostr, const Hi5Tree & group, int & id,
		ODIMPathElem::group_t selector, const ODIMPath & path) {

	drain::Logger mout("DataOutput", __FUNCTION__);

	const std::string fill(path.size()*2, ' ');
	//ostr << "/* '" << path << "' */\n";

	const bool ROOT = (path.empty() || path.back().isRoot());

	//if (!path.empty())
	ostr << "\n/* " << quoted(path) << " */\n";
	//ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";

	/// Draw node
	if (ROOT){
		// Write initial settings
		ostr << '\n';
		//ostr << " size = \"10,8\";\n";
		ostr << "  rankdir=TB;\n"; //  ordering=out;\n";
		ostr << "  ranksep=0.2;\n"; //  ordering=out;\n";
		ostr << "  node [shape=record];\n"; // parallelogram
		ostr << "  tailport=s;\n";
		ostr << '\n';

		ostr << fill << "\"root\"" << " [shape=point];\n"; //" [style=invis][shape=ellipse];\n";

		// Rank fixer
		ostr << fill << 'R'<< 0 << " [style=invis];\n"; //" [style=invis][shape=ellipse];\n";


	}
	else {

		const ODIMPathElem & e = path.back();
		const drain::VariableMap & what  = group["what"].data.attributes;
		const drain::VariableMap & where = group["where"].data.attributes;

		const std::string quantity = what["quantity"].toStr();
		ostr << fill;
		ostr << '"' <<  quoted(path) << '"' << ' ' << '[';

		// LABEL
		ostr << "label=\"";
		ostr << e ; //<< '|';
		if (e.is(ODIMPathElem::DATASET)){
			if (where.hasKey("elangle"))
				ostr << ':' << ' ' << where["elangle"] << "deg" << ' ';
			//ostr << "| {image|attributes}";
		}
		else if (e.belongsTo(ODIMPathElem::DATA|ODIMPathElem::QUALITY)){
			ostr << ':' << ' ' << quantity << ' ';
			// if (group["data"].data.dataSet.isEmpty())
			// ostr << "| {image| }";
		}

		//ostr << "| { <IMG>";
		if (!group.data.dataSet.isEmpty()){
			ostr << "| <IMG>";
			ostr << group.data.dataSet.getGeometry();
		}
		if (!group.data.attributes.empty()){
			ostr << '|' << "<ATTR>";
			ostr << " attributes ";
		}
		//ostr << '}';

		ostr << ' ';
		//ostr << ' ' << ' ' << what["quantity"] << ' ';
		ostr << "\" ";

		// STYLE
		if (group.data.noSave)
			ostr << " style=\"dotted\" ";
		else if (e.isIndexed()){
			ostr << " style=\"filled\"";
		}


		// MISC
		switch (e.group) {
		case ODIMPathElem::DATASET:
			ostr << " fillcolor=\"lightslateblue\"";
			break;
		case ODIMPathElem::DATA:
			ostr << " fillcolor=\"lightblue\"";
			break;
		case ODIMPathElem::QUALITY:
			if ((quantity == "QIND")||(quantity == "CLASS")){
				ostr << " fillcolor=\"darkolivegreen3\"";
			}
			else
				ostr << " fillcolor=\"darkolivegreen1\"";
			break;
		case ODIMPathElem::ROOT:
			ostr << " shape=\"point\"";
			break;
		case ODIMPathElem::ARRAY:
			ostr << " color=\"blue\"";
			break;
		default:
			ostr << " color=\"gray\""; // shape=\"box\"
		}

		ostr << ']' << ';' << '\n';

		ostr << fill << id << "  [shape=point];\n"; //" [shape=ellipse][style=invis];\n";

		/// Draw rank fixer
		ostr << fill << 'R' << id << " [style=invis];\n"; //" [style=invis][shape=ellipse];\n";

	}

	/// Draw egdes
	//ODIMPath prev;
	bool first = true;
	int indexPrev = id;
	for (Hi5Tree::const_iterator it = group.begin(); it!=group.end(); ++it){

		const ODIMPathElem & e = it->first;

		if ((e.group & selector) == 0){  // eg. DATASET, DATA, ARRAY, WHAT, WHERE, HOW
			mout.debug() << "Skipping: " << e << mout.endl;
			continue;
		}

		++id;

		/// Draw rank fixer
		ostr << fill << 'R' <<  (id-1) << " -> R" <<  id << " [style=invis];\n\n"; // [style=invis]

		ODIMPath p(path);  // ,'_')
		//p.push_back(e);
		p << e;
		//mout.note() << "writing: " << p << mout.endl;

		/// Draw edge
		ostr << fill;
		if (first){
			ostr << '"' <<  quoted(path) << '"';
			first = false;
		}
		else {
			ostr << '"' <<  indexPrev << '"';
		}
		ostr << " -> " <<  id << " [arrowhead=none];\n";

		ostr << fill << "{rank=same; R" << id << ";  " << id << "; \"" << quoted(p) << "\" };\n";
		ostr << fill << id << " -> " << '"' << quoted(p) << '"';
		//if (id & 2) ostr <<  " [style=dotted] ";
		ostr << " ;\n"; // [style=invis]
		// TEST
		if (!ROOT)
			//ostr << fill << quoted(p) << ":ATTR -> " << quoted(path) << ":IMG" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
			ostr << fill << quoted(p) << " -> " << quoted(path) << "" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
		ostr << '\n';

		indexPrev = id;

		DataOutput::writeGroupToDot(ostr, it->second, id, selector, p);

	}
	ostr << '\n';

}





}  // rack::

// Rack
// REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP
