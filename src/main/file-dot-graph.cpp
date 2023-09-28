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

	void setAttributes(const init_t & args){
		for (const auto & entry: args){
			attributes[entry.first] = entry.second;
		}
	}

	drain::VariableMap attributes;

	/** Indentation string.
	 */
	static
	const std::string fill;

};

const std::string DotEntity::fill = "  ";


class DotNode : public DotEntity {

public:



	template<typename ... TT>
	DotNode(const TT &... args){
		setName(args...);
	}

	DotNode(init_t args){
		//setName(args...);
		//attributes["label"] = "";
		setAttributes(args);
	}



	template<typename ... TT>
	void setName(const TT &... args){
		std::stringstream sstr;
		_setName(sstr, args...);
		name = sstr.str();
	}

	/// Returns the key of this entity.
	/**
	 *
	 */
	const std::string & getName() const {
		return name;
	}


protected:

	std::string name;

	template<typename T, typename ... TT>
	void _setName(std::stringstream & sstr, const T & arg, const TT &... args){
		sstr << arg;
		_setName(sstr, args...);
	}

	void _setName(std::stringstream & sstr){
	}



	void _setAttributes(std::stringstream & sstr){
		name = sstr.str();
	}


	template<typename T, typename ... TT>
	void _setAttributes(std::stringstream & sstr, const T & arg, const TT &... args){
		sstr << arg;
		_setAttributes(sstr, args...);
		// name = sstr.str();
	}


};

class DotBullit : public DotNode {

public:

	template<typename ... TT>
	DotBullit(const TT &... args) : DotNode(args...){
		setAttributes({{"shape","point"}});
	}

	DotBullit(int index) : DotNode("_B", index){
		setAttributes({{"shape","point"}});
	}

};

class DotRankNode : public DotNode {

public:

	DotRankNode(int rank=0) : DotNode("_R", rank){
		setAttributes({{"style","invis"}});
	}

};



std::ostream & operator<<(std::ostream & ostr, const DotNode & node){
	ostr << DotEntity::fill << '"' << node.getName() << '"';
	if (!node.attributes.empty())
		ostr << ' ' << attrSprintlet(node.attributes);
	ostr << ";\n";
	return ostr;
}

class DotHeader : public DotEntity {

public:


	inline
	DotHeader(const init_t & args = {}, const init_t & nodeArgs =  {}){
		setAttributes(args);
		nodeStyle.setAttributes(nodeArgs);
	}


	void setNodeAttributes(const init_t & args){
		nodeStyle.setAttributes(args);
	}

	inline
	const drain::VariableMap & getNodeAttributes() const {
		return nodeStyle.attributes;
	}

protected:

	//DotNode styleNode;
	DotEntity nodeStyle;

};

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


class DotComment {

public:

	DotComment(const std::string & s = "") : comment(s){
	}

	inline
	const std::string & str() const {
		return comment;
	};

protected:

	std::string comment;

};



std::ostream & operator<<(std::ostream & ostr, const DotComment & comment){
	return ostr << "  /* "  << comment.str() << " */ \n";
}



class DotLink : public DotEntity {

public:

	inline
	DotLink(const DotNode &n1, const DotNode &n2, std::initializer_list<std::pair<const char *,const char *> > args = {}): node1(n1), node2(n2){
		setAttributes(args);
	}

	const DotNode &node1;
	const DotNode &node2;

};

std::ostream & operator<<(std::ostream & ostr, const DotLink & link){
	ostr << DotEntity::fill << '"' << link.node1.getName() << '"' << " -> " << '"' << link.node2.getName() << '"';
	if (!link.attributes.empty())
		ostr << ' ' << attrSprintlet(link.attributes);
	ostr << ";\n";
	return ostr;
}

class DotRank {

public:

	DotRank(const std::string & s = "same") : rank(s){
	}

	inline
	const std::string & str() const {
		return rank;
	};

	inline
	void add(const DotNode & node){
		nodes.push_back(&node);
	}

	std::list<const DotNode *> nodes;


protected:

	std::string rank;

};



std::ostream & operator<<(std::ostream & ostr, const DotRank & rank){
	ostr << DotEntity::fill << "{rank="<< rank.str();
	for (const auto entry: rank.nodes){
		ostr << ' ' << '"' << entry->getName() << '"';
	}
	ostr << "};\n";
	return ostr;
}


void writeGroupToDot(std::ostream & ostr, const Hi5Tree & group, int & index,
		ODIMPathElem::group_t selector, const ODIMPath & path = ODIMPath()) {

	drain::Logger mout(__FUNCTION__, __FILE__);
	// RackContext & ctx = getContext<RackContext>();
	// drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
	// mout.info() << "Dot graph file" << mout.endl;
	// drain::Output ofstr(filename);


	// const std::string fill(path.size()*2, ' ');
	//ostr << "/* '" << path << "' */\n";

	// const bool ROOT = (path.empty() || path.back().isRoot());

	const std::string key = quoted(path);

	// ostr << DotComment(path);

	ostr << DotComment(key);
	// ostr << "\n/* " << key << " */\n";
	//ostr << "/* '" << path << ':' << path.front().group << ':' << path.front().getPrefix() << "' */\n";

	DotNode node(key);

	const ODIMPathElem & e = path.back();

	const drain::FlexVariableMap & attr = group.data.dataSet.properties;

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

	if (!group.data.dataSet.isEmpty()){
		//ostr << "| <IMG>";
		label << group.data.dataSet.getGeometry();
	}
	if (!group.data.attributes.empty()){
		//ostr << '|' << "<ATTR>";
		label << "...";
	}
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

	ostr << node;
	if (index > 0)
		ostr << DotBullit(index);
	/// Draw rank fixer   [style=invis]
	ostr << DotRankNode(index);
	//ostr << fill << 'R' << id << INVISIBLE << ";\n"; //" [style=invis][shape=ellipse];\n";

	//}


	/// Draw egdes
	bool first = true;
	int indexPrev = index;
	for (const auto & entry: group){

		const ODIMPathElem & e = entry.first;

		if ((e.group & selector) == 0){  // eg. DATASET, DATA, ARRAY, WHAT, WHERE, HOW
			mout.debug() << "Skipping: " << e << mout.endl;
			continue;
		}

		++index;

		/// Draw rank fixer
		//ostr << fill << 'R' <<  (id-1) << " -> R" <<  id << ' '<< INVISIBLE << ";\n\n"; // [style=invis]

		DotRankNode rankNode(index);

		ostr << DotLink(DotRankNode(index-1), rankNode, {{"style", "invis"}}); // [style=invis]

		ODIMPath p(path);  // , _')
		//p.push_back(e);
		p << e;
		const std::string subkey = quoted(p);
		// mout.note("writing: ", p);

		DotBullit idNode(index);

		/// Draw edge
		// ostr << fill;
		if (first){
			// parentNode.setName(key);
			//ostr << '"' <<  key << '"';
			ostr << DotLink(node, idNode, {{"arrowhead","none"}});
			first = false;
		}
		else {
			// DotBullit idNodePrev(indexPrev);
			//parentNode.setName('B' ,indexPrev);
			//ostr << '"' <<  indexPrev << '"';
			ostr << DotLink(DotBullit(indexPrev), idNode, {{"arrowhead","none"}});
		}
		// ostr << " -> " <<  id << ARROWLESS << ";\n"; //  [arrowhead=none]

		DotNode subNode(subkey);

		// ostr << fill << "{rank=same; R" << id << ";  " << id << "; \"" << subkey << "\" };\n";
		DotRank rank;
		rank.add(rankNode);
		rank.add(idNode);
		rank.add(subNode);

		ostr << rank;
		// ostr << "{rank=same; " << rankNode.getName() << ";  " << idNode.getName() << "; \"" << subNode.getName() << "\" };\n";

		ostr << DotLink(idNode, subNode);

		// Display variable collector (TOO DETAILED!)
		/*
			if (!ROOT)
				//ostr << fill << quoted(p) << ":ATTR -> " << quoted(path) << ":IMG" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
				ostr << fill << quoted(p) << " -> " << quoted(path) << "" << " [weight=0.1, style=dotted, color=gray] ;\n"; // [style=invis]
		 */

		ostr << '\n';

		indexPrev = index;

		writeGroupToDot(ostr, entry.second, index, selector, p);

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

	ostr << DotComment("header");
	DotHeader header({
		{"rankdir", "TB"},
		{"ranksep", "0.2"},
		{"tailport", "s"}});
	header.setNodeAttributes({{"shape","box"}});
	ostr << header;

	writeGroupToDot(output, src, index, selector);
	ostr << "}\n";

}



} // namespace rack
