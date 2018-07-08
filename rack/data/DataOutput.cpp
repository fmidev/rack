/*

    Copyright 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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

		ostr << "label=\"<H>|<T>";
		ostr << e << '|';
		if (e.group == BaseODIM::DATASET){
			if (where.hasKey("elangle"))
				//ostr << ' ' << '(' << where["elangle"] << ')';
				ostr << where["elangle"];
		}
		else if (e.group & BaseODIM::DATA) // includes BaseODIM::QUALITY
			ostr << what["quantity"];
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

		//const HI5TREE & g = it->second;
		ODIMPath p(path);
		p.push_back(e);

		if (prev.empty() && !prev.back().isRoot()){
			ostr << "/** '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";
			if (!ROOT){ // or path.
				ostr << fill;
				ostr << '"' << path << '"' << ":T";
				ostr << "\t -> " << '"' << p  << '"' << ":H" << ';' << '\n';
			}
			// ELSE ???
		}
		else {
			ostr << fill;
			ostr << '"'<< prev << '"'<< ":H"; // fix to prev?
			ostr << "\t -> " << '"'<< p << '"' << ":H" << ';' << '\n';
		}
		DataOutput::writeGroupToDot(ostr, it->second, id, selector, p);
		prev = p;
	}
	ostr << '\n';

}





}  // rack::
