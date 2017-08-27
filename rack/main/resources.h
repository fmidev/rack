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

#pragma once
#ifndef RACK_RESOURCES_H_
#define RACK_RESOURCES_H_

#include <data/ODIM.h>
//#include <drain/image/Coordinates.h>
//#include <drain/image/PaletteOp.h>
//#include <drain/prog/CommandPack.h>
//#include <drain/prog/Commands-ImageTools.h>
//#include <drain/util/Debug.h>
//#include <drain/util/ReferenceMap.h>
#include <hi5/Hi5.h>
#include <image/Coordinates.h>
#include <image/Image.h>
#include <image/PaletteOp.h>
#include <image/Sampler.h>
#include <prog/CommandPack.h>
#include <prog/CommandRegistry.h>
#include <radar/Composite.h>
#include <radar/RadarAccumulator.h>
#include <util/Rectangle.h>
#include <util/RegExp.h>
#include <util/Tree.h>
//#include "product/CumulativeProductOp.h"
//#include "product/VolumeOp.h"
//#include <map>
#include <string>


namespace rack {


/// A container for shared resources applied by RackLets.
/**
 *
 */
class RackResources {

public:

	RackResources(); // : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL), currentGrayImage(NULL) {};

	// STATUS FLAGS

	/// True, if the last input file operation has been successful. Helps in skipping operations for null data.
	bool inputOk;

	/// True, if the last retrieved data was found (and non-empty?). Helps in skipping operations for null data.
	bool dataOk;

	/// True, if the last retrieved data was found (and non-empty?). Helps in skipping operations for null data.
	//bool productOk;

	//typedef enum {ANDRE=1, PROD=2, ACCUMULATOR=4} SOURCE_CODE;


	void setSource(HI5TREE & dst, const drain::Command & cmd);


	/// The last input file read, typically a volume. May be concatenated ie. read in incrementally.
	HI5TREE inputHi5;
	// SOURCE inputHi5src;

	/// The polar product that has been generated from the volume.
	HI5TREE polarHi5;
	/// Flag for marking the origin of polarHi5
	//SOURCE  polarHi5src;

	/// A single-radar Cartesian product or a multi-radar composite (mosaic).
	HI5TREE cartesianHi5;
	/// Flag for marking the origin of cartesianHi5
	//SOURCE  cartesianHi5src;

	/// Pointer to the last HDF5 structure read or generated.
	HI5TREE *currentHi5; // = &inputHi5;

	/// Pointer to the last HDF5 structure in polar coordinates: input volume or polar product.
	/**
	 *   The data pointed to will be applied as input to a Cartesian product (single-radar or composite).
	 */
	HI5TREE *currentPolarHi5; // = &inputHi5;

	static
	const CoordinatePolicy polarLeft;

	static
	const CoordinatePolicy limit;

	//static bool inputOk;

	static
	void updateCoordinatePolicy(HI5TREE & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));

	drain::VariableMap & getUpdatedStatusMap();

	drain::image::Image grayImage;
	drain::image::Image colorImage;

	const drain::image::Image *currentImage; // = NULL;
	const drain::image::Image *currentGrayImage; // = NULL;  // data or grayImage
	drain::image::Palette palette;  // Todo: drainize


	RadarAccumulator<Accumulator,PolarODIM> polarAccumulator;

	Composite composite;
	// double cDefaultQuality; // why separate? replaced with composite.defaultQuality,

	drain::Rectangle<double> bbox;
	std::string projStr;


	/// Global values accessible more or less directly through commands.

	/* FILE I/O  */
	int inputSelect; // see fileio.cpp and hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS

	/// Path prefix for input files.
	std::string inputPrefix;

	/// Path prefix for output files.
	std::string outputPrefix;

	std::string targetEncoding;

	std::string select;
	std::string andreSelect;


	drain::ScriptParser scriptParser;
	drain::ScriptExec   scriptExec;

	drain::image::Sampler sampler;

};

/// Default instance
RackResources & getResources();


template <class T>
class RackLetAdapter : public T {
    public: //re 


	RackLetAdapter(const std::string & key = "", char alias = 0){ // : Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(const std::string & key, char alias, const T & value) : T(value) { // : Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(char alias){
		add("", alias);
	}

	void add(const std::string & key, char alias){

		const std::string & k = key.empty() ? T::getName() : key;

		static RegExp nameCutter("^(Cmd|Cartesian|Composite|Polar)(.*)$");

		if (nameCutter.execute(k) == 0){ // matches
			//std::cerr << "adding () " << nameCutter.result[2] << std::endl;
			getRegistry().add(*this, nameCutter.result[2], alias);
		}
		else {
			getRegistry().add(*this, k, alias);
			//std::cerr << "adding    " << k << std::endl;
		}

	}
	/*
	const std::string & getName() const {
		return T::getName();
	}

	const std::string & getDescription() const {
		return T::getDescription();
	}
	*/


};




} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
