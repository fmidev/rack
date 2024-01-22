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

//#include <exception>
#include <fstream>
#include <iostream>

#include "drain/util/Log.h"
#include "drain/util/DotGraph.h"
#include "drain/util/Output.h"
#include "drain/image/Sampler.h"

#include "radar/RadarDataPicker.h"

#include "fileio.h"
#include "resources.h"



//drain::SprinterLayout dotAttributeLayout("*","[ ]", "=", "\"\"");



/*
typedef std::pair<const char*, drain::Variable> vmap_pair_t;
//typedef std::pair<std::string, drain::Variable> vmap_pair_t;
typedef std::initializer_list<vmap_pair_t> init_list_t;
*/

/*
template <class T>
drain::Sprintlet<std::initializer_list<T> > sprinter(const std::initializer_list<T> & x, const drain::SprinterLayout & layout = dotAttributeLayout){
	return drain::Sprintlet<T>(drain::VariableMap(x), layout); // copy const
}
*/

/*
class tester : public drain::Sprintlet<drain::VariableMap> {
public:
	tester(const drain::VariableMap & m) : drain::Sprintlet<drain::VariableMap>(m, dotAttributeLayout) {};

	tester(const init_list_t & l) : drain::Sprintlet<drain::VariableMap>(l, dotAttributeLayout) {};

};
*/



namespace rack {


std::string quoted(const ODIMPath & path){
	//std::stringstream sstr;
	if (path.empty() || path.back().isRoot())
		return "root";

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::Path<std::string,'_'> p(path);
	mout.debug2(p.str() );

	return p.str();

}

class typeIsFundamental {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){ return std::is_fundamental<S>::value; }

};



void writeGroupToDot(std::ostream & ostr, const Hi5Tree & group, int & index,
		ODIMPathElem::group_t selector, const ODIMPath & path = ODIMPath()) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// RackContext & ctx = getContext<RackContext>();
	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	// mout.info("Dot graph file" );
	// drain::Output ofstr(filename);


	// const std::string fill(path.size()*2, ' ');
	//ostr << "/* '" << path << "' */\n";

	// const bool ROOT = (path.empty() || path.back().isRoot());

	const std::string key = quoted(path);

	// ostr << DotComment(path);

	ostr << drain::DotComment(key);
	// ostr << "\n/* " << key << " */\n";
	//ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";

	drain::DotNode node(key);

	const ODIMPathElem & e = path.back();

	const drain::FlexVariableMap & attr = group.data.image.properties;

	const std::string quantity = attr.get("what:quantity", ""); // what["quantity"].toStr();

	// LABEL
	drain::Variable & label = node.attributes["label"];
	label.setType(typeid(std::string)); // MUST!
	label.setSeparator(0);
	label = e;
	//ostr << "label=\"";
	//ostr << e ; //<< '|';
	if (e.is(ODIMPathElem::DATASET)){
		if (attr.hasKey("where:elangle"))
			label << ':' << ' ' << attr["where:elangle"] << "deg" << ' ';
	}
	else if (e.belongsTo(ODIMPathElem::DATA|ODIMPathElem::QUALITY)){
		label << ':' << ' ' << quantity << ' ';
	}

	if (!group.data.image.isEmpty()){
		//ostr << "| <IMG>";
		label << group.data.image.getGeometry();
	}
	if (!group.data.attributes.empty()){
		//ostr << '|' << "<ATTR>";
		label << "...";
	}
	// END LABEL

	// STYLE
	if (group.data.exclude)
		node.attributes["style"] = "dotted"; //ostr << " style=\"dotted\" ";
	else if (e.isIndexed()){
		node.attributes["style"] = "filled"; // ostr << " style=\"filled\"";
	}

	// MISC
	switch (e.group) {
	case ODIMPathElem::DATASET:
		node.attributes["fillcolor"] = "lightslateblue"; // ostr << " fillcolor=\"lightslateblue\"";
		break;
	case ODIMPathElem::DATA:
		node.attributes["fillcolor"] = "lightblue"; // ostr << " fillcolor=\"lightblue\"";
		break;
	case ODIMPathElem::QUALITY:
		if ((quantity == "QIND")||(quantity == "CLASS")){
			node.attributes["fillcolor"] = "darkolivegreen3"; //ostr << " fillcolor=\"darkolivegreen3\"";
		}
		else
			node.attributes["fillcolor"] = "darkolivegreen1"; //ostr << " fillcolor=\"darkolivegreen1\"";
		break;
	case ODIMPathElem::ROOT:
		node.attributes["shape"] = "point"; // ostr << " shape=\"point\"";
		break;
	case ODIMPathElem::ARRAY:
		node.attributes["color"] = "blue"; //ostr << " color=\"blue\"";
		break;
	default:
		node.attributes["color"] = "gray"; //ostr << " color=\"gray\""; // shape=\"box\"
	}

	ostr << node;
	if (index > 0)
		ostr << drain::DotBullit(index);
	/// Draw rank fixer   [style=invis]
	ostr << drain::DotRankNode(index);


	/// Draw egdes
	int indexPrev = index;
	bool FIRST = true;
	for (const auto & entry: group){

		const ODIMPathElem & e = entry.first;

		if ((e.group & selector) == 0){  // eg. DATASET, DATA, ARRAY, WHAT, WHERE, HOW
			mout.debug("Skipping: " , e );
			continue;
		}

		++index;

		/// Draw rank fixer
		drain::DotRankNode rankNode(index);

		ostr << drain::DotLink(drain::DotRankNode(index-1), rankNode, {{"style", "invis"}}); // [style=invis]

		ODIMPath p(path);  // , _')
		p << e;
		//const std::string subkey = quoted(p);

		drain::DotBullit idNode(index);

		/// Draw edge
		if (FIRST){
			ostr << drain::DotLink(node, idNode, {{"arrowhead","none"}});
			FIRST = false;
		}
		else
			ostr << drain::DotLink(drain::DotBullit(indexPrev), idNode, {{"arrowhead","none"}});

		drain::DotNode subNode(p);

		drain::DotRank rank;
		rank.add(rankNode);
		rank.add(idNode);
		rank.add(subNode);
		ostr << rank;

		ostr << drain::DotLink(idNode, subNode);
		ostr << '\n';

		indexPrev = index;

		// Note: \c index can grow several steps here:
		writeGroupToDot(ostr, entry.second, index, selector, p);

	}
	ostr << '\n';


}



void CmdOutputFile::writeDotGraph(const Hi5Tree & src, const std::string & filename, ODIMPathElem::group_t selector) const {  //  = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED)

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.unimplemented("Attribute selection, like in --outputTree");

	int index = 0;

	drain::Output output(filename);

	std::ostream & ostr = output;

	ostr << "digraph G { \n";

	ostr << "/* group selector (mask)=" << selector << " */  \n"; // consider escaping

	ostr << drain::DotComment("header");
	drain::DotHeader header(
		// Graph
		{
			{"rankdir", "TB"},
			{"ranksep", 0.2},
			{"tailport", "s"}
		},
		// Node:
		{
			{"shape","box"}
		}
	);

	// header.setNodeAttributes({{"shape","box"}});
	ostr << header;

	writeGroupToDot(output, src, index, selector);
	ostr << "}\n";

}



} // namespace rack
