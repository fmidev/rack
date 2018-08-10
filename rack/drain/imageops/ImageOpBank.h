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

// Drain
