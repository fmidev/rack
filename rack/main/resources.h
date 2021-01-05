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

// # pragma once
#ifndef RACK_RESOURCES_H_
#define RACK_RESOURCES_H_

#include <string>

#include "drain/image/Image.h"
#include "drain/imageops/ImageModifierPack.h"
#include "drain/imageops/PaletteOp.h"
#include "drain/prog/CommandPack.h"

#include "drain/prog/CommandRegistry.h" // OLD
//#include "drain/prog/CommandBank.h"     // NEW

#include "drain/util/Rectangle.h"
#include "drain/util/RegExp.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"

#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"

#include "radar/Composite.h"
#include "radar/RadarAccumulator.h"


namespace rack {


// Resources provided separately for each thread.
class RackContext : public drain::Context {

public:

	RackContext();

	/// The last input file read, typically a volume. May be concatenated ie. read in incrementally.
	Hi5Tree inputHi5;
	// SOURCE inputHi5src;

	/// The polar product that has been generated from the volume.
	Hi5Tree polarHi5;
	/// Flag for marking the origin of polarHi5
	//SOURCE  polarHi5src;

	/// A single-radar Cartesian product or a multi-radar composite (mosaic).
	Hi5Tree cartesianHi5;

	/// Pointer to the last HDF5 structure read or generated.
	Hi5Tree *currentHi5; // = &inputHi5;

	/// Pointer to the last HDF5 structure in polar coordinates: input volume or polar product.
	/**
	 *   The data pointed to will be applied as input to a Cartesian product (single-radar or composite).
	 */
	Hi5Tree *currentPolarHi5; // = &inputHi5;


};

/// A container for shared resources applied by RackLets.
/**
 *
 */
class RackResources : public RackContext {

public:

	RackResources(); // : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL), currentGrayImage(NULL) {};


	/// Clears dst if source command varies.
	void setSource(Hi5Tree & dst, const drain::Command & cmd);



	/// Standard (?) orientation of polar coords in radar data
	static
	const CoordinatePolicy polarLeft;

	/// Default coordinate policy; no wrapping or mirroring at edges.
	static
	const CoordinatePolicy limit;

	//static bool inputOk;

	//static 	void updateCoordinatePolicy(Hi5Tree & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));

	drain::VariableMap & getUpdatedStatusMap();

	/// Retrieves image that matches a given selector.
	/*
	 *  \return true, if non-empty image was found
	 */
	// bool
	ODIMPath setCurrentImage(const DataSelector & imageSelector);


	// Experimental
	bool guessDatasetGroup(ODIMPathElem & pathElem) const;
	// Experimental
	ODIMPath currentPath;


	drain::image::Image grayImage;
	drain::image::Image colorImage;

	/// Pointer to the last 2D data (image) processed
	const drain::image::Image *currentImage; // = NULL;

	/// Pointer to the last single-channel 2D data (image) processed
	const drain::image::Image *currentGrayImage; // = NULL;  // data or grayImage


	drain::image::Palette palette;  // Todo: drainize
	//drain::image::PaletteOp;

	// Accumulator for data in polar coordinates
	RadarAccumulator<Accumulator,PolarODIM> polarAccumulator;

	// Accumulator for data in Cartesian coordinates
	Composite composite;

	void initComposite();

	drain::Rectangle<double> bbox;

	std::string projStr;

	/// Global values accessible more or less directly through commands.

	//  FILE I/O
	// int inputSelect; // see fileio.cpp and hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS

	/// Path prefix for input files.
	std::string inputPrefix;

	/// Path prefix for output files.
	std::string outputPrefix;

	std::string targetEncoding;

	std::string select;
	std::string andreSelect;


	drain::ScriptParser scriptParser;
	drain::ScriptExec   scriptExec;

	drain::image::ImageSampler sampler; // could be in ImageModPack?




	static const drain::Flags::value_t INPUT_ERROR;//     = 1;
	static const drain::Flags::value_t DATA_ERROR;//      = 2;
	static const drain::Flags::value_t METADATA_ERROR;//  = 4;
	static const drain::Flags::value_t OUTPUT_ERROR;//    = 8;
	static const drain::Flags::value_t PARAMETER_ERROR;// = 16;
	//static const drain::Flags::value_t IO_ERROR;// = INPUT_ERROR | OUTPUT_ERROR;

	drain::Flags2 errorFlags; //(value, dict, ',');

protected:


	// void getImageInfo(const char *label, const drain::image::Image *ptr, VariableMap & statusMap);
	void getImageInfo(const drain::image::Image *ptr, drain::Variable & entry) const;

};

/// Default instance
RackResources & getResources();



//typedef drain::BeanCommand<image::ImageOp, image::ImageOp &> ImageOpAdapter;


/// Adapter for commands designed for Rack.
/**
 *    \tparam T - class derived from drain::Command
 */
template <class T>
class RackLetAdapter : public T {

public:


	RackLetAdapter(const std::string & key = "", char alias = 0){ // : drain::Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(const std::string & key, char alias, const T & value) : T(value) { // : drain::Command("cart", name, alias) {
		add(key, alias);
	};

	RackLetAdapter(char alias){
		add("", alias);
	}

	void add(const std::string & key, char alias){

		const std::string & k = key.empty() ? T::getName() : key;

		static drain::RegExp nameCutter("^(Cmd|Cartesian|Composite|Polar)(.*)$");

		if (nameCutter.execute(k) == 0){ // matches
			//std::cerr << "adding () " << nameCutter.result[2] << std::endl;
			drain::getRegistry().add(*this, nameCutter.result[2], alias);
			// drain::getCommandBank().addExternal(nameCutter.result[2], alias, *this);
			// creates new: drain::getCommandBank().add<T>(nameCutter.result[2], alias);
		}
		else {
			drain::getRegistry().add(*this, k, alias);
			// drain::getCommandBank().addExternal(k, alias, *this);
			// std::cerr << "adding    " << k << std::endl;
		}

	}

	inline
	void setParameters(const std::string & args, char assignmentSymbol='='){
		T::setParameters(args, assignmentSymbol);
	}

	inline
	void setParameters(const drain::VariableMap & p){
		T::setParameters(p);
	};

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
