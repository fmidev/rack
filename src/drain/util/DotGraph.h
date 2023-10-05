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
#ifndef DOT_GRAPH_H_
#define DOT_GRAPH_H_

// #include <pair>
#include "VariableMap.h"


namespace drain
{


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


class DotEntity {

public:

	typedef std::initializer_list<std::pair<const char *,const Variable> > init_t;
	// typedef std::initializer_list<std::pair<const char *,const char *> > init_t;


	void setAttributes(const init_t & args);

	drain::VariableMap attributes;

	/** Indentation string.
	 */
	static
	const std::string fill;

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



std::ostream & operator<<(std::ostream & ostr, const DotNode & node);

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

std::ostream & operator<<(std::ostream & ostr, const DotHeader & header);


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



std::ostream & operator<<(std::ostream & ostr, const DotComment & comment);


class DotLink : public DotEntity {

public:

	inline
	DotLink(const DotNode &n1, const DotNode &n2, init_t args = {}): node1(n1), node2(n2){
		setAttributes(args);
	}

	const DotNode &node1;
	const DotNode &node2;

};

std::ostream & operator<<(std::ostream & ostr, const DotLink & link);

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



std::ostream & operator<<(std::ostream & ostr, const DotRank & rank);



} // ::drain

#endif

