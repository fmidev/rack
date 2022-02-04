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

#ifndef RACK_FILE_IO
#define RACK_FILE_IO

#include <string>

#include "drain/util/RegExp.h"
#include "drain/prog/CommandBank.h"
#include "drain/prog/CommandInstaller.h"

#include "drain/util/FileInfo.h"


#include "hi5/Hi5.h"


namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.
extern
const drain::RegExp h5FileExtension;

/// Syntax for recognising GeoTIFF files.
extern
//const drain::RegExp tiffFileExtension;
const drain::FileInfo fileInfoTIFF;

/// Syntax for recognising Portable Networks Image image file.
//extern
//const drain::RegExp pngFileExtension;

/// Syntax for recognising PNM (PGM,PPM) image file.
//extern
//const drain::RegExp pnmFileExtension;

/// Syntax for recognising text files.
extern
const drain::RegExp textFileExtension;

/// Syntax for recognising numeric array files (in plain text format anyway).
extern
const drain::RegExp arrayFileExtension;

/// Syntax for sparsely resampled data.
extern
const drain::RegExp sampleFileExtension;

class CmdOutputFile : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdOutputFile() : drain::SimpleCommand<>(__FUNCTION__, "Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.",
			"filename", "", "<filename>.[h5|hdf5|png|pgm|txt|dat|mat|dot]|-") {
	};

	void writeProfile(const Hi5Tree & src, const std::string & filename) const;

	void writeSamples(const Hi5Tree & src, const std::string & filename) const;

	void writeDotGraph(const Hi5Tree & src, const std::string & filename, ODIMPathElem::group_t selector = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED)) const;

	void exec() const;

protected:

	/**
	 *  \tparam P - Picker class (PolarDataPicker or CartesianDataPicker)
	 */
	/*
	template <class P>
	void sampleData(const typename P::dataset_t & dataset, const Sampler & sampler, const std::string & format, std::ostream &ostr) const {

		RackContext & ctx  = this->template getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		P picker(sampler.variableMap, dataset.getFirstData().odim);

		typename P::map_t dataMap;

		for (typename DataSet<typename P::src_t>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
			dataMap.insert(typename P::map_t::value_type(it->first, it->second));
		}

		const typename P::data_t & q = dataset.getQualityData();
		if (!q.data.isEmpty()){
			mout.note() << "using quality data, quantity=" << q.odim.quantity << mout.endl;
			if (q.odim.quantity.empty()){
				mout.warn() << " empty data, properties: \n " <<  q.data.properties  << mout.endl;
			}
			dataMap.insert(typename P::map_t::value_type(q.odim.quantity, q));
		}
		else {
			mout.info() << "no quality data" << mout.endl;
		}

		sampler.sample(dataMap, picker, format, ostr);

	}
	*/


};



class FileModule : public drain::CommandModule<> { // : public drain::CommandGroup {

public:

	FileModule(drain::CommandBank & bank = drain::getCommandBank());
	//virtual void initialize();

};

}

#endif

// Rack
