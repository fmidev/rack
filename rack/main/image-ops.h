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


#ifndef RACK_IMAGE_OPS
#define RACK_IMAGE_OPS

#include <list>
#include <string>

#include "drain/imageops/ImageOp.h"
#include <drain/prog/CommandInstaller.h>
//#include "drain/util/ReferenceMap.h"


#include "resources.h"



//drain::BeanRefAdapter<drain::image::ImageOp>

//typedef drain::BeanRefCommand<drain::image::ImageOp> ImageOpCmd;


namespace rack {


class ImageOpExec {


public:

	/// Shared function, to minimize copies in template classes.
	void execOp(const drain::image::ImageOp & imageOp, RackContext & ctx) const;

	// static bool physical; //? or just next!  MOVE

	//static std::string outputQuantitySyntax; // MOVE

	template <class OD>
	void updateGeometryODIM(Hi5Tree & dstGroup, const std::string & quantity, drain::image::Geometry & geometry) const {

		drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout;

		// OD odim;
		typedef DstType<OD> dst_t;

		DataSet<dst_t> dstDataSet(dstGroup, quantity);


		for (typename DataSet<dst_t>::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){

			Data<dst_t> & d = dit->second;
			if (geometry.getArea() == 0){
				geometry.setArea(d.data.getWidth(), d.data.getHeight());
			}
			else if (geometry != d.data.getGeometry()) {
				mout.warn() << "dataset group contains different geometries" << d.odim << mout.endl;
			}

			d.odim.setGeometry(d.data.getWidth(), d.data.getHeight());
			//mout.note() << "modified odim geom " << d.odim << mout.endl;


		}

		//ODIM::copyToH5<ODIMPathElem::ROOT>(rootODIM, dstH5);

	}




};






/// Directive. Set physical scale on or off.
/**
 *   Applied also by CartesianGrid
 */
class CmdPhysical : public drain::SimpleCommand<bool> {

public:

	CmdPhysical() :  drain::SimpleCommand<bool>(__FUNCTION__,
			"Flag. Handle intensities as physical quantities instead of storage typed values.", "physical", false){
		//parameters.link("value", ImageOpExec::physical);
	};

	virtual
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.imagePhysical = value;
	}


};



struct ImageOpSection : public drain::CommandSection {
	inline	ImageOpSection(): CommandSection("imageOps"){
		drain::CommandBank::trimWords().insert("Functor"); // Functor
	};
};


template <class OP>
class RackImageOpCmd;


class ImageOpModule : public drain::CommandModule<'i',ImageOpSection> {
	//: public drain::CommandSection { // : public drain::CommandGroup {

public:

	ImageOpModule();

	/// Add ImageOp command to registry (CommandBank).
	// \tparam OP - Class derived from ImageOp
	template <class OP>
	void install(const std::string & name = OP().getName()){

		try {

			std::string key(name);
			drain::CommandBank::deriveCmdName(key, getPrefix());

			drain::Command & cmd = cmdBank.add<RackImageOpCmd<OP> >(key);
			cmd.section = getSection().index;
		}
		catch (const std::exception &e) {
			std::cerr << "error: ImageOpInstaller: " << name  << '\n';
			std::cerr << "error: " << e.what() << '\n';
		}
	}

};


} /* namespace rack */

#endif

// Rack
