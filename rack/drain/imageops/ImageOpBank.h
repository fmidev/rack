/**

    Copyright 2017 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef DRAIN_IMAGE_OP_BANK_H_
#define DRAIN_IMAGE_OP_BANK_H_

//
#include <iostream>
#include <map>

//#include "util/Log.h"

//#include "util/Registry.h"
//#include "util/Cloner.h"
#include "util/Bank.h"
#include "util/FunctorPack.h"
#include "util/RegExp.h"

#include "ImageOp.h"
#include "FunctorOp.h"


// using namespace std;

namespace drain
{

namespace image
{

template <class T>
class ImageOpCloner : public Cloner<ImageOp,T> {
public:

	static
	const std::string & getName(){
		static T op;
		return op.getName();
	}

};

template <class F>
class UnaryFunctorOpCloner : public ImageOpCloner<UnaryFunctorOp<F> > {
    public:
};


template <class F>
class BinaryFunctorOpCloner : public ImageOpCloner<BinaryFunctorOp<F> > {
    public:
};






class ImageOpBank : public Bank<ImageOp> {

public:

	virtual
	~ImageOpBank(){}

	template <class T>
	void add(T & op, const std::string & name){
		Bank<ImageOp>::add(op, name);
	}

	template <class T>
	void add(T & op){

		std::string name = op.getName();
		StringTools::lowerCase(name, 1);
		//name[0] = (name[0]+'a')-'A';

		static drain::RegExp nameCutter("^(.*)(Op|Functor)$");
		if (nameCutter.execute(name) == 0){
			//std::cerr << name << '>' << nameCutter << std::endl;
			Bank<ImageOp>::add(op, nameCutter.result[1]);
		}
		else {
			Bank<ImageOp>::add(op, name);
		}

	}

	/// Supports querying operator with parameters set, eg. gaussianAverage,width=10,height=5
	ImageOp & getComplete(const std::string & query, char separator=',', char assign='=', const drain::SmartMap<std::string> & aliasMap = drain::SmartMap<std::string>());

};



std::ostream & operator<<(std::ostream & ostr, const ImageOpBank & bank);


// consider separate
extern
ImageOpBank & getImageOpBank();


}

}

#endif
