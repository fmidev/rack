/**


    Copyright 2014 - 2015   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.


*/


#ifndef RACK_IMAGES
#define RACK_IMAGES

#include <drain/image/Image.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandAdapter.h>

#include "data/DataSelector.h"


#include "resources.h"
#include "commands.h" // cmdSelect

namespace drain {

typedef BeanRefAdapter<drain::image::ImageOp> ImageOpAdapter;

}


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

		//drain::Logger & mout = resources.mout;
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
	void convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters,
			drain::image::Image &dst);


};
extern CommandEntry<CmdImage> cmdImage;

/**
 *   Applied also by CartesianGrid
 */
class CmdPhysical : public drain::SimpleCommand<bool> {

public:

	CmdPhysical() : drain::SimpleCommand<bool>(__FUNCTION__, "Handle intensities as physical quantities like dBZ (instead of that of storage type).",
			"value", true, "0,1") {
	};


};
extern CommandEntry<CmdPhysical> cmdPhysical;


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
	ImageOpRacklet(const ImageOpRacklet & a) : ImageOpAdapter(a.bean), key(a.key) {
		//imageOp.getParameters().updateFromMap(a.imageOp.getParameters());
	}

	virtual
	void exec() const;

	/// Name of this operator, to be recognized
	const std::string key;

	static std::string outputQuantity;

protected:

};


class ImageRackletModule : public CommandGroup {
public:

	typedef std::list<ImageOpRacklet> list_t;

	static
	list_t rackletList;

	ImageRackletModule(const std::string & section = "image", const std::string & prefix = "i");

};


} /* namespace rack */

#endif
