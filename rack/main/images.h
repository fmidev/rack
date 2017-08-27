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


#ifndef RACK_IMAGES
#define RACK_IMAGES

#include <drain/image/Image.h>
#include <drain/prog/Command.h>
#include <drain/prog/Commands-ImageOps.h>

#include "data/DataSelector.h"


#include "resources.h"
#include "commands.h" // cmdSelect

namespace rack {

/**
 *   Applied also by CartesianGrid
 */
class CmdImage : public drain::BasicCommand {
    public: //re 
	mutable DataSelector imageSelector;

	CmdImage() : BasicCommand(__FUNCTION__, "Copies data to a separate image object. Encoding can be changed with --target ."), imageSelector(".*/data/?$","") {
	};

	inline
	void exec() const {

		//drain::MonitorSource & mout = resources.mout;
		RackResources & resources = getResources();
		imageSelector.setParameters(resources.select);
		resources.select.clear();

		convertImage(*resources.currentHi5, imageSelector, resources.targetEncoding, resources.grayImage);
		resources.targetEncoding.clear();
		//convertImage(*getResources().currentHi5, imageSelector, properties, getResources().grayImage);

		resources.currentGrayImage = & resources.grayImage;
		resources.currentImage     = & resources.grayImage;
		//File::write(*resources.currentImage, "convert.png");
	};

	static
	void convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters, drain::image::Image &dst);


};
extern CommandEntry<CmdImage> cmdImage;

/// Designed for Rack
class ImageOpRacklet : public drain::ImageOpAdapter {

public:

	/**
	 *  \param op - image operator to be used
	 *  \param key - command name
	 */
	ImageOpRacklet(drain::ImageOp & imageOp, const std::string & key) : ImageOpAdapter(imageOp), key(key) {
	};

	ImageOpRacklet(drain::ImageOp & imageOp) : ImageOpAdapter(imageOp), key(imageOp.getName()) {
	};


	/// Copy constructor.
	ImageOpRacklet(const ImageOpRacklet & a) : ImageOpAdapter(a.imageOp), key(a.key) {
		//imageOp.getParameters().updateFromMap(a.imageOp.getParameters());
	}

	virtual
	void exec() const;

	// static
	// void populate(CommandRegistry & registry);

	const std::string key;

	static std::string outputQuantity;

protected:

	/// Traverse the structure and process image data, saving results with <quantity_new> = <quantity>_<cmd> .
	/*
	 *  \tparam T
	 */
	template <class T>
	void processDataSet(HI5TREE & root, const std::string & path, MonitorSource & mout) const {

		Data<SrcType<T> > srcData(root(path));
		const drain::image::Image & src = srcData.data;

		RackResources & resources = getResources();


		if (!src.isEmpty()){
			mout.note() << " processing '" << srcData.odim.quantity << " (" << path << ')' << mout.endl;
			mout.info() << this->imageOp.getName() << " (" << this->imageOp.getParameters() << ')' << mout.endl;
			DataSetDst<DstType<T> > dstDataSet(root(DataSelector::getParent(path)));

			std::string quantityDst;
			if (false){ // future option if empty() // TODO
				//resources.updateStatusMap();
				drain::StringMapper formatter("[a-zA-Z0-9_:]+");
				formatter.parse("{what:quantity}_"+key);
				quantityDst = formatter.toStr(srcData.odim.getMap());
				/*
				std::map<std::string, std::string> m;
				m["key"]    = key;
				m["params"] = this->imageOp.getParameters().toStr();
				formatter.expand(m);
				formatter.expand(srcData.odim.getMap());
				*/
				// std::stringstream sstr;
				// sstr << formatter;
				// quantityDst = sstr. str();
			}
			else {
				// quantityDst = srcData.odim.quantity+"_" + key;
				quantityDst = srcData.odim.quantity;
			}


			Data<DstType<T> > & dstData = dstDataSet.getData(quantityDst); //this->imageOp.getName()); // consider cmdName
			drain::image::Image & dst = dstData.data;
			// TODO 1: use quality field
			// TODO 2: Data<>::setGeometry()
			const drain::image::Geometry originalGeometry(dst.getGeometry());
			this->imageOp.filter(src, dst);
			// TODO dstData.updateTree();
			// TODO DataSelector::updateAttributes(root(path));
			if (dst.getGeometry() != originalGeometry){
				mout.warn() << " geometry changed: "<< originalGeometry << " => "<< dst.getGeometry() << mout.endl;
			}

			resources.currentGrayImage = & dst;
			resources.currentImage     = & dst;
		}
		else
			mout.note() << "skipping path: " << path << mout.endl;

	}

};

class ImageRackletModule : public CommandGroup {
public:

	ImageRackletModule(const std::string & section = "image", const std::string & prefix = "i");

};


} /* namespace rack */

#endif

// Rack
