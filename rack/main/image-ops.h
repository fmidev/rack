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
#include "drain/prog/Command.h"
#include "drain/prog/CommandAdapter.h"
#include "drain/prog/CommandRegistry.h"
#include "drain/util/ReferenceMap.h"


#include "resources.h"


namespace drain {

//drain::BeanRefAdapter<drain::image::ImageOp>

typedef drain::BeanCommand<image::ImageOp, image::ImageOp &> ImageOpAdapter;
//BeanCommand<B, B &>
}


namespace rack {



/// Designed for Rack
class ImageOpRacklet : public drain::ImageOpAdapter {

public:

	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used
	 *  \param key - command name
	 */
	ImageOpRacklet(drain::image::ImageOp & imageOp, const std::string & key) : drain::ImageOpAdapter(imageOp), key(key) {
	};


	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used througjh reference
	 */
	ImageOpRacklet(drain::image::ImageOp & imageOp) : drain::ImageOpAdapter(imageOp), key(imageOp.getName()) {
	};


	/// Copy constructor.
	ImageOpRacklet(const ImageOpRacklet & a) : drain::ImageOpAdapter(a.bean), key(a.key) {
		//imageOp.getParameters().updateFromMap(a.imageOp.getParameters());
	}

	void setParameters(const std::string & args, char assignmentSymbol='='){
		bean.setParameters(args, assignmentSymbol);
	};

	//void setParameters(const drain::SmartMap<T> & args){
	void setParameters(const drain::VariableMap & args){
		bean.setParameters(args);
		//productOp.setParameters((const drain::SmartMap<T> &) args);
	};

	virtual
	void exec() const;

	/// Name of this operator, to be recognized
	const std::string key;

	static std::string outputQuantity;

	static bool physical;

protected:

	// --iResize may have changed size
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

protected:

};


/**
 *   Applied also by CartesianGrid
 */
class CmdPhysical : public drain::BasicCommand { //SimpleCommand<bool> {

public:

	CmdPhysical() :  drain::BasicCommand(__FUNCTION__, "Flag. Handle intensities as physical quantities instead of storage typed values."){
		//drain::SimpleCommand<bool>(__FUNCTION__, "Handle intensities as physical quantities like dBZ (instead of that of storage type).",
		//	"value", true, "0,1")

		parameters.link("value", ImageOpRacklet::physical, "false|true");

	};


};


class CmdPaletteOut : public drain::SimpleCommand<> {

public:

	CmdPaletteOut() : drain::SimpleCommand<>(__FUNCTION__, "Save palette as TXT, JSON or SVG.", "filename", "") {
	};

	void exec() const;

};




class ImageRackletModule : public drain::CommandGroup {

public:

	typedef std::list<ImageOpRacklet> list_t;

	static
	list_t rackletList;


	ImageRackletModule(const std::string & section = "image", const std::string & prefix = "i");

};


} /* namespace rack */

#endif

// Rack
