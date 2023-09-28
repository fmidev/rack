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



#include "DotGraph.h"


namespace drain
{


void DotEntity::setAttributes(const init_t & args){
	for (const auto & entry: args){
		attributes[entry.first] = entry.second;
	}
}


const std::string DotEntity::fill = "  ";






std::ostream & operator<<(std::ostream & ostr, const DotNode & node){
	ostr << DotEntity::fill << '"' << node.getName() << '"';
	if (!node.attributes.empty())
		ostr << ' ' << attrSprintlet(node.attributes);
	ostr << ";\n";
	return ostr;
}


std::ostream & operator<<(std::ostream & ostr, const DotHeader & header){

	for (const auto & entry: header.attributes){
		ostr << DotEntity::fill << entry.first << " = " << entry.second << ";\n";
	}

	if (!header.getNodeAttributes().empty()){
		ostr << DotEntity::fill << "node " << attrSprintlet(header.getNodeAttributes()) << ";\n";
	}
	ostr << std::endl;

	return ostr;
}





std::ostream & operator<<(std::ostream & ostr, const DotComment & comment){
	return ostr << "  /* "  << comment.str() << " */ \n";
}




std::ostream & operator<<(std::ostream & ostr, const DotLink & link){
	ostr << DotEntity::fill << '"' << link.node1.getName() << '"' << " -> " << '"' << link.node2.getName() << '"';
	if (!link.attributes.empty())
		ostr << ' ' << attrSprintlet(link.attributes);
	ostr << ";\n";
	return ostr;
}



std::ostream & operator<<(std::ostream & ostr, const DotRank & rank){
	ostr << DotEntity::fill << "{rank="<< rank.str();
	for (const auto entry: rank.nodes){
		ostr << ' ' << '"' << entry->getName() << '"';
	}
	ostr << "};\n";
	return ostr;
}


} // namespace drain
