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

#include <set>
#include <map>
#include <ostream>

#include "util/Debug.h"
#include "util/Proj4.h"
#include "util/RegExp.h"


#include "image/Image.h"
#include "image/File.h"

#include "image/Sampler.h"

//  ******  NEW scheme ******

#include "Command.h"
#include "CommandRegistry.h"
#include "CommandPack.h"




#include "Commands-ImageTools.h"



namespace drain {

using namespace image;

void CmdSampler::setReferences(){
	parameters.reference("iStep",  sampler.iStep = 10, "horz coord step");
	parameters.reference("jStep",  sampler.jStep =  0, "vert coord step");
	parameters.reference("iStart", sampler.iStart = -1, "horz coord start");
	parameters.reference("jStart", sampler.jStart = -1, "vert coord start");
	parameters.reference("iEnd",   sampler.iEnd = -1,  "horz coord end");
	parameters.reference("jEnd",   sampler.jEnd = -1,  "vert coord end");
	parameters.reference("commentChar",   sampler.commentChar = "#",  "comment character");
	parameters.reference("skipVoid", sampler.skipVoid = 0,  "skip lines with invalid/missing values");
}



void CmdCoordPolicy::filter(Image & dst) const {
	drain::MonitorSource mout("CmdCoordPolicy", __FUNCTION__);
	//mout.note() << "file:" << value << mout.endl;

	//Drainage & r = getDrainage();
	//Image & dst = r.getSrc();

	CoordinatePolicy p;
	std::vector<int> v;
	Variable(value, typeid(int)).toVector(v);
	switch (v.size()) {  // switch (v.getElementCount()) {
	case 4:
		//dst.setCoordinatePolicy(v.get<int>(0), v.get<int>(1),v.get<int>(2),v.get<int>(3));
		p.yOverFlowPolicy  = v[3];
		p.xOverFlowPolicy  = v[2];
		p.yUnderFlowPolicy = v[1];
		p.xUnderFlowPolicy = v[0];
		break;
	case 2:
		p.yOverFlowPolicy  = v[1];
		p.xOverFlowPolicy  = v[0];
		p.yUnderFlowPolicy = v[1];
		p.xUnderFlowPolicy = v[0];
		break;
	case 1:
		p.yOverFlowPolicy  = v[0]; //.get<int>(0);
		p.xOverFlowPolicy  = v[0];
		p.yUnderFlowPolicy = v[0];
		p.xUnderFlowPolicy = v[0];
		break;
	default:
		mout.error() << "Wrong number of parameters (not 1, 2 or 4): " << v.size() << " (" << value << ")" << mout.endl;
		break;
	}
	dst.setCoordinatePolicy(p);
}








void CmdPlot::filter(Image & dst) const {
	drain::MonitorSource mout(name, __FUNCTION__);
	mout.note() << value << mout.endl;

	//Drainage & r = getDrainage();
	//Image & dst = r.getSrc();

	Variable p;
	p.setType<double>();
	p = value;
	std::vector<double> v;
	p.toVector(v);
	int n = std::min(v.size() - 2, dst.getChannelCount());
	int i = v[0];
	int j = v[1];
	const CoordinateHandler2D coordHandler(dst.getWidth(), dst.getHeight(), dst.getCoordinatePolicy());
	coordHandler.handle(i, j);
	for (int k=0; k<n; ++k)
		dst.put(i, j, k,  v[2+k]);

}



void CmdPlotFile::filter(Image & dst) const {

	drain::MonitorSource mout(name, __FUNCTION__);
	mout.note() << "file: " << value << mout.endl;

	//Drainage & r = getDrainage();
	//Image & dst = r.getDst();

	std::ifstream ifstr;
	ifstr.open(value.c_str());
	//read(ifstr);
	std::istream &istr = ifstr; // std::cin;

	std::string line;
	double x;
	double y;
	double d = 0.0;
	const size_t n = dst.getChannelCount();
	std::stringstream sstr;
	while ( !getline(istr,line).eof() ){

		//if ((line.length() > 0) && line.at(0))
		if (line.at(0) != '%'){
			sstr.str(line);
			sstr >> x >> y;
			for (size_t k=0; k<n; k++){
				sstr >> d;
				if (sstr.eof())
					break;
				dst.put<double>(x,y,k,d);
			}
			std::cout << x << ',' << y << '\t' << d << '\n';
			//std::cout << i << ' ' << x << '\t' << y << '\n';
		}
		//ifstr.get
	}

	ifstr.close();

}


//CommandEntry<CmdSample> cmdSample("sample");






/*
DrainageImageTools::DrainageImageTools(const std::string & section, const std::string & prefix) : CommandGroup(section, prefix) {
// void DrainageImageTools::populate() {

	static CommandEntry<CmdChannels> cmdChannels;
	static CommandEntry<CmdCoordPolicy> cmdCoordPolicy;
	static CommandEntry<CmdFill> fill;
	static CommandEntry<CmdGeometry> geometry;
	static CommandEntry<CmdImageSampler> sampler("sample");
	static CommandEntry<CmdSetScale> setScale("scale");
	static CommandEntry<CmdView> cmdView("view",'V');

}
*/

} // drain::


// Rack
