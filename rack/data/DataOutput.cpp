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


#include "BaseODIM.h"
#include "DataOutput.h"

namespace rack {

//using namespace hi5;


void DataOutput::writeGroupToDot(std::ostream & ostr, const HI5TREE & group, int & id,
		BaseODIM::group_t selector, const ODIMPath & path) {

	const std::string fill(path.size()*2, ' ');
	//ostr << "/* '" << path << "' */\n";

	const bool ROOT = (path.empty() || path.back().isRoot());

	if (!path.empty())
		ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";


	if (ROOT){
		ostr << '\n';
		//ostr << "  size = \"10,8\";\n";
		ostr << "  rankdir=TB;\n"; //  ordering=out;\n";
		ostr << "  ranksep=0.2;\n"; //  ordering=out;\n";
		ostr << "  node [shape=record];\n";
		ostr << "  tailport=s;\n";
		ostr << '\n';
	}
	else {

		const ODIMPathElem & e = path.back();
		const drain::VariableMap & what  = group["what"].data.attributes;
		const drain::VariableMap & where = group["where"].data.attributes;

		ostr << fill;
		ostr << '"' << path << '"' << ' ' << '[';

		//ostr << "label=\"<H>|<T>"; // referring to "head" "tail"
		ostr << "label=\"";
		ostr << e ; //<< '|';
		if (e.group == BaseODIM::DATASET){
			if (where.hasKey("elangle"))
				ostr << ' ' << '(' << where["elangle"] << '\xB0' << ')';
				//ostr << where["elangle"];
		}
		else if (e.group & BaseODIM::DATA) // includes BaseODIM::QUALITY
			ostr << ' ' << '[' << what["quantity"] << ']';
		//ostr << ' ' << ' ' << what["quantity"] << ' ';
		ostr << "\" ";

		// color & fill style
		if (e.group & BaseODIM::IS_INDEXED){
			ostr << " style=\"filled\"";
			switch (e.group) {
			case BaseODIM::DATASET:
				ostr << " fillcolor=\"lightslateblue\"";
				break;
			case BaseODIM::DATA:
				ostr << " fillcolor=\"lightblue\"";
				break;
			case BaseODIM::QUALITY:
				ostr << " fillcolor=\"lightseagreen\"";
				break;
			default:
				ostr << " fillcolor=\"lightyellow\"";
			}
		}
		else { // WHAT, WHERE, HOW, DATA(array)
			ostr << " color=\"gray\"";
		}

		ostr << ']' << ';' << '\n';
		ostr << fill << "{rank=same; " << id << "; \"" << path << "\" };\n";
		ostr << fill << id << " [style=invis];\n";
	}


	ODIMPath prev;
	for (HI5TREE::const_iterator it = group.begin(); it!=group.end(); ++it){

		ODIMPathElem e(it->first);
		//ostr << "/* '" << e << '|' << (int)e.group << ',' << selector << "' */\n";

		if ((e.group & selector) == 0){  // DATASET, DATA, ARRAY, WHAT, WHERE, HOW
			continue;
		}

		++id;
		if (id > 1){
			ostr << fill<< (id-1) << " -> " << id << " [style=invis];\n\n";
		}

		ODIMPath p(path);
		p.push_back(e);

		/// Draw edge
		if (prev.empty() && !prev.back().isRoot()){
			// ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";
			if (!ROOT){ // or path.
				ostr << fill;
				ostr << '"' << path << '"'; // << ":T";
				ostr << "\t -> " << '"' << p  << '"';// << ":H"
				ostr << "  [tailport=s headport=w]" <<  ';' << '\n';
			}
			// ELSE ???
		}
		else {
			ostr << fill;
			ostr << '"'<< prev << '"'; //<< ":H"; // fix to prev?
			ostr << "\t -> " << '"'<< p << '"' << ';' << '\n'; // << ":H" << ';' << '\n';
		}
		DataOutput::writeGroupToDot(ostr, it->second, id, selector, p);
		prev = p;
	}
	ostr << '\n';

}





}  // rack::

// Rack
