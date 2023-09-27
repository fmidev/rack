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
#include "drain/util/Output.h"
#include "drain/image/Sampler.h"

#include "radar/RadarDataPicker.h"

#include "fileio.h"
#include "resources.h"



//drain::SprinterLayout dotAttributeLayout("*","[ ]", "=", "\"\"");


class DotLayout : public drain::SprinterLayout {

public:

	inline
	DotLayout() : SprinterLayout("*","[ ]", "=", "\"\""){};

	// inline
	// DotLayout(const DotLayout & layout) : SprinterLayout("*","[ ]", "=", "\"\""){};

};

typedef drain::Sprintlet<drain::VariableMap, DotLayout> attrSprintlet;
typedef std::map<std::string,std::string> amap_t;  //map_t m2 = {{"koe", 123.45}};
typedef drain::Sprintlet<amap_t, DotLayout> AttrSprintlet;


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

	drain::Logger mout(__FUNCTION__, __FILE__);

	drain::Path<std::string,'_'> p(path);
	mout.debug2() << p.str() << mout.endl;

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

class DotEntity {

public:

	typedef std::initializer_list<std::pair<const char *,const char *> > init_t;

	void configure(const init_t & args){
		for (const auto & entry: args){
			attributes[entry.first] = entry.second;
		}
	}

	drain::VariableMap attributes;

};


class DotNode : public DotEntity {

public:



	template<typename ... TT>
	DotNode(const TT &... args){
		setName(args...);
	}

	DotNode(init_t args){
		//setName(args...);
		//attributes["label"] = "";
		configure(args);
	}


	void _configure(std::stringstream & sstr){
		name = sstr.str();
	}


	template<typename T, typename ... TT>
	void _configure(std::stringstream & sstr, const T & arg, const TT &... args){
		sstr << arg;
		_configure(sstr, args...);
		// name = sstr.str();
	}



	template<typename ... TT>
	void setName(const TT &... args){
		std::stringstream sstr;
		_setName(sstr, args...);
		name = sstr.str();
	}


	template<typename T, typename ... TT>
	void _setName(std::stringstream & sstr, const T & arg, const TT &... args){
		sstr << arg;
		_setName(sstr, args...);
	}

	void _setName(std::stringstream & sstr){
	}

	std::string name;

};

class DotBullit : public DotNode {

public:

	template<typename ... TT>
	DotBullit(const TT &... args) : DotNode(args...){
		configure({{"shape","point"}});
	}

	DotBullit(int index) : DotNode('B', index){
		configure({{"shape","point"}});
	}

};

class DotRankNode : public DotNode {

public:

	DotRankNode(int rank=0) : DotNode('R', rank){
		configure({{"style","invis"}});
	}

};



std::ostream & operator<<(std::ostream & ostr, const DotNode & node){
	ostr << "  " << '"' << node.name << '"';
	if (!node.attributes.empty())
		ostr << ' ' << attrSprintlet(node.attributes);
	ostr << ";\n";
	return ostr;
}



class DotLink : public DotEntity {

public:

	inline
	DotLink(const DotNode &n1, const DotNode &n2, std::initializer_list<std::pair<const char *,const char *> > args = {}): node1(n1), node2(n2){
		configure(args);
	}

	const DotNode &node1;
	const DotNode &node2;

};

std::ostream & operator<<(std::ostream & ostr, const DotLink & link){
	ostr << "  " << '"' << link.node1.name << '"' << " -> " << '"' << link.node2.name << '"';
	if (!link.attributes.empty())
		ostr << ' ' << attrSprintlet(link.attributes);
	ostr << ";\n";
	return ostr;
}


void writeGroupToDot(std::ostream & ostr, const Hi5Tree & group, int & id,
		ODIMPathElem::group_t selector, const ODIMPath & path = ODIMPath()) {

	drain::Logger mout(__FUNCTION__, __FILE__);
	// RackContext & ctx = getContext<RackContext>();
	// drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
	// mout.info() << "Dot graph file" << mout.endl;
	// drain::Output ofstr(filename);

	DotNode test1({{"color","red"}});
	test1.configure({{"color","red"}});

	DotNode test2;
	test2.configure({{"shape","box"}, {"arrowhead","none"}});
	test2.setName('R', 234, "name");
	// test2.configure('R', 234, "name", {{}});

	//test2.configure("miks", {{"shape","box"}, {"arrowhead","none"}});

	// mout.note("writing: ", id);

	/*
	static const std::string BOX       = attrSprintlet({{"shape","box"}}).str();
	static const std::string POINT     = attrSprintlet({{"shape","point"}}).str();
	static const std::string INVISIBLE = attrSprintlet({{"style","invis"}}).str();
	static const std::string ARROWLESS = attrSprintlet({{"arrowhead","none"}}).str();
	*/

	const std::string fill(path.size()*2, ' ');
	//ostr << "/* '" << path << "' */\n";

	const bool ROOT = (path.empty() || path.back().isRoot());

	const std::string key = quoted(path);

	//if (!path.empty())
	ostr << "\n/* " << key << " */\n";
	//ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";

	DotNode node(key);

	/// Draw node
	if (ROOT){

		DotBullit root("root");
		// Write initial settings
		ostr << '\n';
		//ostr << " size = \"10,8\";\n";
		ostr << "  rankdir=TB;\n"; //  ordering=out;\n";
		ostr << "  ranksep=0.2;\n"; //  ordering=out;\n";
		// ostr << "  node [shape=point];\n"; // box caused problems in new dot versions
		//ostr << "  node " << BOX << ";\n"; // [shape=box] parallelogram
		ostr << "  node [shape=box];\n"; //  parallelogram
		ostr << "  tailport=s;\n";
		ostr << '\n';

		ostr << fill << root << '\n';
		// ostr << fill << "\"root\" " << POINT << " ;\n"; //" [style=invis][shape=ellipse];\n";

		// Rank fixer
		// ostr << fill << 'R' << 0 << ' ' << INVISIBLE << ";\n"; //" [style=invis][shape=ellipse];\n";
		// DotRankNode rankNode(0);
		ostr << DotRankNode(0);

	}
	else {

		const ODIMPathElem & e = path.back();

		const drain::FlexVariableMap & attr = group.data.dataSet.properties;

		const std::string quantity = attr.get("what:quantity", ""); // what["quantity"].toStr();
		ostr << fill;

		// ostr << '"' <<  '/' << quoted(path) << '"' << ' ' << '[';
		// ostr << '"' <<  key << '"' << ' '; //  << '[';
		// node.setName(key);

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
				//ostr << ':' << ' ' << attr["where:elangle"] << "deg" << ' ';
		}
		else if (e.belongsTo(ODIMPathElem::DATA|ODIMPathElem::QUALITY)){
			//std::cerr << what << std::endl;
			//group.data.dataSet.properties.get("what:quantity", "");
			label << ':' << ' ' << quantity << ' ';
			//ostr << ':' << ' ' << group.data.dataSet.properties.get("what:quantity", "") << ' ';
			// if (group["data"].data.dataSet.isEmpty())
			// ostr << "| {image| }";
		}

		//ostr << "| { <IMG>";
		if (!group.data.dataSet.isEmpty()){
			//ostr << "| <IMG>";
			label << group.data.dataSet.getGeometry();
		}
		if (!group.data.attributes.empty()){
			//ostr << '|' << "<ATTR>";
			//ostr << " attributes ";
			label << "...";
		}

		// ostr << ' ';
		// ostr << "\" ";
		// END LABEL

		// STYLE
		if (group.data.noSave)
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

		ostr << node;  // << ']' << ';' << '\n';

		ostr << DotBullit(id);
		// ostr << fill << id << ' ' << POINT <<";\n"; //" [shape=ellipse][style=invis];\n";

		/// Draw rank fixer   [style=invis]
		ostr << DotRankNode(id);
		//ostr << fill << 'R' << id << INVISIBLE << ";\n"; //" [style=invis][shape=ellipse];\n";

	}


	/// Draw egdes
	bool first = true;
	int indexPrev = id;
	for (const auto & entry: group){

		const ODIMPathElem & e = entry.first;

		if ((e.group & selector) == 0){  // eg. DATASET, DATA, ARRAY, WHAT, WHERE, HOW
			mout.debug() << "Skipping: " << e << mout.endl;
			continue;
		}

		++id;

		/// Draw rank fixer
		//ostr << fill << 'R' <<  (id-1) << " -> R" <<  id << ' '<< INVISIBLE << ";\n\n"; // [style=invis]

		DotRankNode rankNode(id);

		ostr << fill << DotLink(DotRankNode(id-1), rankNode, {{"style", "invis"}}); // [style=invis]

		ODIMPath p(path);  // , _')
		//p.push_back(e);
		p << e;
		const std::string subkey = quoted(p);
		// mout.note("writing: ", p);

		DotBullit idNode(id);

		/// Draw edge
		// ostr << fill;
		if (first){
			// parentNode.setName(key);
			//ostr << '"' <<  key << '"';
			DotLink link(node, idNode);
			link.configure({{"arrowhead","none"}});
			ostr << fill << link;
			first = false;
		}
		else {
			// DotBullit idNodePrev(indexPrev);
			//parentNode.setName('B' ,indexPrev);
			//ostr << '"' <<  indexPrev << '"';
			DotLink link(DotBullit(indexPrev), idNode);
			link.configure({{"arrowhead","none"}});
			ostr << fill << link;
		}
		// ostr << " -> " <<  id << ARROWLESS << ";\n"; //  [arrowhead=none]

		DotNode subNode(subkey);

		// ostr << fill << "{rank=same; R" << id << ";  " << id << "; \"" << subkey << "\" };\n";
		ostr << fill << "{rank=same; " << rankNode.name << ";  " << idNode.name << "; \"" << subNode.name << "\" };\n";

		ostr << fill << DotLink(idNode, subNode);

		// Display variable collector (TOO DETAILED!)
		/*
			if (!ROOT)
				//ostr << fill << quoted(p) << ":ATTR -> " << quoted(path) << ":IMG" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
				ostr << fill << quoted(p) << " -> " << quoted(path) << "" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
		 */

		ostr << '\n';

		indexPrev = id;

		writeGroupToDot(ostr, entry.second, id, selector, p);

	}
	ostr << '\n';


}



void CmdOutputFile::writeDotGraph(const Hi5Tree & src, const std::string & filename, ODIMPathElem::group_t selector) const {  //  = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED)

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.unimplemented("Attribute selection, like in --outputTree");

	int index = 0;

	drain::Output output(filename);

	std::ostream & ostr = output;

	ostr << "digraph G { \n";
	ostr << "/* group selector (mask)=" << selector << " */  \n"; // consider escaping
	writeGroupToDot(output, src, index, selector);
	ostr << "}\n";

}



} // namespace rack
