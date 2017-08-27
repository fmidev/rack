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
#ifndef ODIM_STRUCT
#define ODIM_STRUCT

#include <ostream>
#include <cmath>
#include <string>
#include <set>
//#include <drain/util/Options.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/Rectangle.h>
#include <drain/util/Time.h>

#include "hi5/Hi5.h"
#include "radar/Constants.h"

namespace rack {

class SourceODIM : public drain::ReferenceMap {

public:

	std::string source;
	std::string WMO;
	std::string RAD;
	std::string NOD;
	std::string PLC;
	std::string ORG;
	std::string CTY;
	std::string CMT;

	/// Sets NOD, WMO, RAD, CTY and ORG
	/**
	 *   \param source - ODIM source std::string separated with ':', eg. "what:source=WMO:02870,RAD:FI47,PLC:Utajärvi,NOD:fiuta"
	 *
	 *   Note: some radars have had semicolon as separator: RAD:NL51;PLC:nldhl
	 */
	inline
	SourceODIM(const std::string & source = "") : source(source) {
		init();
		setValues(source, ':');
		setNOD();
	};


	inline
	SourceODIM(const SourceODIM & s){
		init();
		updateFromMap(s);
		setNOD();
	};

	/// Sets NOD, WMO, RAD, CTY and ORG
	// ----

	/// Derives a most standard name. Returns the first-non empty value of WMO, RAD, NOD, PLC, ORG. CTY, CMT.
	const std::string & getSourceCode() const;

private:

	//inline
	void init();

	/// Assigns NOD if empty, and CMT
	//inline
	void setNOD();

};




/**
 *  Within each ODIM class, each variable should have
 *
 *  -# name
 *  -# type
 *  -# group/name
 *  -# dataset/group/name
 *
 *  The scope for variables is "down to the data array". Ie.
 *
 *  Common interface:
 *
 *  -# operator[](toStr)
 *  -#
 *
 *  Class specific members:
 *
 *  -# native variables (double, long int, std::string)
 *
 *  Typically used in creating and writing a product.
 *
 */
class EncodingODIM : public drain::ReferenceMap {

public:


	EncodingODIM(){ init(); };

	EncodingODIM(const EncodingODIM & odim){
		init();
		updateFromMap(odim); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
	};

	EncodingODIM & operator=(const EncodingODIM & odim) {
		// std::cerr << "EncodingODIM & operator=" << std::endl;
		updateFromMap(odim);
		return *this;
	}

	/// This is non-standard (not in ODIM), but a practical means of handling storage type of datasets.
	//  See drain::Type.
	std::string type;

	/// data[n]/what (obligatory)
	double gain;
	double offset;
	double nodata;
	double undetect;

	EncodingODIM & setScaling(double gain, double offset = NAN);

	EncodingODIM & setScaling(double gain, double offset, double undetect, double nodata);

	inline
	bool isSet() const {
		return ((gain != 0.0) && (!type.empty()) && (type.at(0) != '*'));
	}

	/// Checks if data encoding is similar (storage type, gain, offset, undetect and nodata are the same).
	// todo: bool operator==(const EncodingODIM & odim);
	static
	inline
	bool haveSimilarEncoding(const EncodingODIM & odim1, const EncodingODIM & odim2){
		return  (odim1.type == odim2.type) &&
				(odim1.gain == odim2.gain) &&
				(odim1.offset == odim2.offset) &&
				(odim1.undetect == odim2.undetect) &&
				(odim1.nodata == odim2.nodata)
				;
	};

	void setRange(double min, double max);  // todo rename setRange

	/// Sets gain=1, offset=0, undetect=type_min, nodata=type_max. Sets type, if unset.
	inline
	void setTypeDefaults(const std::string & type = "", const std::string & values = ""){

		if (this->type.empty())
			this->type = type;

		gain = 1.0;
	    offset = 0.0;

	    if (!type.empty()){ // ?
	    	undetect = drain::Type::getMin<double>(type);
	    	nodata   = drain::Type::getMax<double>(type);
	    }
	    else {
	    	undetect = drain::Type::getMin<double>(this->type);
	    	nodata   = drain::Type::getMax<double>(this->type);
	    }

		setValues(values);

	}

	/// Converts a quantity from storage scale: y = offset + gain*y .
	inline
	double scaleForward(double x) const {
		return offset + gain*x;
	}

	/// Converts a quantity to storage scale: x = (y-offset)/gain .
	inline
	double scaleInverse(double y) const {
		return (y-offset)/gain;
	}

	/// Returns the minimum value that can be returned using current storage type, gain and offset.
	inline
	double getMin() const {

		if (type.empty())
			throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

		if (drain::Type::isIntegralType(type)){
			long int i = drain::Type::getMin<long int>(type);
			if (static_cast<double>(i) != undetect)
				return scaleForward(static_cast<double>(i));
			else
				return scaleForward(static_cast<double>(i+1));
		}
		else
			return scaleForward(drain::Type::getMin<double>(type));

	}

	inline
	double getMax() const {

		if (type.empty())
			throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

		if (drain::Type::isIntegralType(type)){
			long int i = drain::Type::getMax<long int>(type);
			if (static_cast<double>(i) != undetect)
				return scaleForward(static_cast<double>(i));
			else
				return scaleForward(static_cast<double>(i-1));
		}
		else
			return scaleForward(drain::Type::getMax<double>(type));

	}


	/// Functor (why inverse?)
	inline
	double operator()(double y) const {
		return (y-offset)/gain;
	}


	/// Resets the values.
	// ?virtual?
	void clear();


	/// Todo: keep the function, but move implementation to (future single-exec) register ?
	/*
	inline
	void getShortKeys(std::map<std::string,std::string> & m){
		 for (ReferenceMap::iterator it = begin(); it != end(); ++it){
			 const std::string & longKey = it->first;
			 const std::string  shortKey = longKey.substr(0,longKey.find(':'));
			 m[shortKey] = longKey;
			 std::cerr << shortKey << '#' << longKey << '\n';
		 }
	}
	*/


	/// Sets type, if unset (empty).
	virtual
	void update(const EncodingODIM & odim);


	/// Creates a short alias \c (attrib) for each \c (group):(attrib). Example: "gain" => "what:gain".
	/*  The object itself can be given as an argument.
	 *	Does not change the keys of the object.
	 */
	void addShortKeys(drain::ReferenceMap & ref);

	/// Copies image attributes and \b type . Experimental.
	/**
	 *  Image storage type, "type", is a non-standard property that must be copied separately.
	 */
	void copyFrom(const drain::image::Image & data);

	/// Traverses recursively subtrees and checks the types of PolarODIM variables.
	/**
	 *  \param dst - start group
	 */
	static
	inline
	void checkType(HI5TREE & dst){ EncodingODIM odim; checkType(dst, odim); } // Temp for thread-safety.


	/// Copies contents of this to a h5 group.
	/**
	 *  \param dstGroup - target group under which the respective \c /what , \c /where , and \c /how groups will be populated directly.
	 *
	 *  Currently, uses direct sub groups only, which means that subgroups referring to /dataset??/data??/data are used only.
	 *  \todo Change "what:quantity" -style namings to "dataset:data:what:quantity" .
	 *
	 *  dataset1/data1/quality1
	 */

	/// Write top-level ODIM data, eg. \c /what
	void copyToRoot(HI5TREE & dst) const { copyTo(EncodingODIM::getRootAttributes(), dst);};

	/// Write ODIM data relevant for dataset level, eg. \c /dataset2/what
	// virtual
	inline
	void copyToDataSet(HI5TREE & dst) const { copyTo(EncodingODIM::getDatasetAttributes(), dst);};

	/// Write ODIM data relevant for data level, eg. \c /dataset2/what
	//virtual
	inline
	void copyToData(HI5TREE & dst) const { copyTo(EncodingODIM::getDataAttributes(), dst);};

	// Slow, because gets initialized for every object. C++ has no static initialization.
	inline
	static std::set<std::string> & getRootAttributes(){
		static std::set<std::string> rootAttributes;
		return rootAttributes;
	}


	// Slow, because gets initialized for every object. C++ has no static initialization.
	inline
	static std::set<std::string> & getDatasetAttributes(){
		static std::set<std::string> datasetAttributes;
		return datasetAttributes;
	}

	// Slow, because gets initialized for every object. C++ has no static initialization.
	inline
	static std::set<std::string> & getDataAttributes(){
		static std::set<std::string> dataAttributes;
		return dataAttributes;
	}


	/// A set containing "what", "where" and "how".
	static
	const std::set<std::string> & attributeGroups;


protected:


	virtual
	void init();


	template <class F>
	void declare(std::set<std::string> & keys, const std::string & key, F &x){
		keys.insert(key);
		reference(key, x);
	}


	///
	void copyTo(const std::set<std::string> & keys, HI5TREE & dst) const;

	static
	void checkType(HI5TREE & dst, EncodingODIM & odim); // SEE ABOVE?


private:

	static
	const std::set<std::string> & createAttributeGroups();




};


/// ODIM metadata (quantity, gain, offset, undetect, nodata, date, time)
class ODIM : public EncodingODIM {

public:

	ODIM(){ init(); };

	ODIM(const ODIM & odim){
		init();
		updateFromMap(odim);
	};

	~ODIM(){};

	/// /what (obligatory)
	std::string object;
	std::string version;

	std::string date;
	std::string time;
	std::string source;

	/// dataX/what (obligatory)
	std::string quantity;  // raise?
	std::string product;   // raise?
	std::string prodpar;   // raise?
	std::string startdate;
	std::string starttime;
	std::string enddate;
	std::string endtime;

	// Maximum Nyquist velocity
	double NI;

	/// Updates object, quantity, product and time information.
	/*!
	 *  Fills empty values. Updates time variables.
	 */
	virtual
	void update(const ODIM & odim);

	/// Retrieves the stored time. Returns true if successful.
	bool getTime(drain::Time & t) const;

	/// If nodata==undetect, set nodata=maxValue (hoping its not nodata...)
	/**
	 *  This oddity is needed because some manufactures do not distinguish between undetect and nodata (esp. with VRAD).
	 *
	 *  \param quantity - set only if this quantity applies.
	 */
	inline
	bool distinguishNodata(const std::string & quantity = ""){
		if (quantity.empty() || (quantity == this->quantity)){  // Fix Vaisala IRIS bug
			//std::cerr << "setNodata" << quantity << '\n';
			if (nodata == undetect){
				nodata = drain::Type::getMax<double>(type);
				return true;
			}
			//std::cerr << "nodata: " << nodata << '\n';
		}
		return false;
	}

	/// For VRAD, set encoding range to cover [-NI,NI]
	/**
	 *  \return - true if quantity was VRAD
	 */
	inline
	bool optimiseVRAD(){
		if (quantity == "VRAD"){
			setRange(-NI, NI);
			return true;
		}
		else
			return false;
	}


protected:

	virtual void init();



};

/// Metadata structure for single-radar data (polar scans, volumes and products).
/**
 *
 */
//
class PolarODIM : public ODIM {


public:

	PolarODIM(){ init(); };

	PolarODIM(const PolarODIM & odim){
		init();
		updateFromMap(odim);
	};

	PolarODIM(const drain::image::Image & image){
		init();
		drain::MonitorSource mout(__FUNCTION__,__FUNCTION__);
		copyFrom(image);
		//mout.warn() << EncodingODIM(*this) << mout.endl;
	};



	/// Number of range bins in each ray
	long   nbins;
	long   nrays;
	double rscale;


	// datasetX/where:
	/// Longitude position of the radar antenna (degrees), normalized to the WGS-84 reference ellipsoid and datum. Fractions of a degree are given in decimal notation.
	double lon;
	/// Latitude position of the radar antenna (degrees), normalized to the WGS-84 reference ellipsoid and datum. Fractions of a degree are given in decimal notation.
	double lat;
	/// Height of the centre of the antenna in meters above sea level.
	double height;

	/// Antenna elevation angle (degrees) above the horizon.
	double elangle;
	/// The range (km) of the start of the first range bin.
	double rstart;
	/// Index of the first azimuth gate radiated in the scan.
	long   a1gate;


	double startaz;
	double stopaz;

	//double NI; // Maximum Nyquist
	double highprf; //
	double lowprf;  //
	double wavelength;

	/// Freezing level
	double freeze;

	/// Returns the distance in metres to the start of the measurement volume (i.e. the end nearer to radar).
	/*
	inline
	double getDistance(long binIndex) const {
		return rstart + static_cast<double>(binIndex)*rscale;
	};
	*/

	inline
	void setGeometry(size_t cols, size_t rows){
		nbins = cols;
		nrays = rows;
	}

	/// Returns the distance along the beam to the center of the i'th bin.
	inline
	double getBinDistance(size_t i) const {
		return rstart + (static_cast<double>(i)+0.5)*rscale;
	}

	/// Returns the index of bin at given (bin center) distance along the beam.
	inline
	int getBinIndex(double d) const {
		return static_cast<int>((d-rstart)/rscale) ;
	}

	/// Returns the index of a ray at a given azimuth [radians].
	inline
	int getRayIndex(double d) const {
		return static_cast<int>(d*static_cast<double>(nrays)/(2.0*M_PI)) ;
	}

	/// Returns the azimuth in radians of the bin with vertical index j.
	template <class T>
	inline
	double getAzimuth(T j) const {
		return static_cast<double>(j)*2.0*M_PI / static_cast<double>(nrays);
	}

	/// Returns the span of bins for the given azimuthal span.
	inline
	int getAzimuthalBins(double degree) const {
		return static_cast<int>(degree * static_cast<double>(nrays)/360.0 + 0.5) ;
	}

	/// Returns the span of bins for the given distance range in meters.
	inline
	int getBeamBins(double spanM) const {
		return static_cast<int>(spanM/rscale + 0.5) ;
	}

	/// Returns the range in metres (i.e. distance to the end of the last measurement volume).
	inline
	double getMaxRange() const {
		return rstart + static_cast<double>(nbins)*rscale;
		//return getBinDistance(nbins);
	};


	double getGroundAngle(size_t i) const {
		return (static_cast<double>(i)+0.5) * rscale / EARTH_RADIUS_43;
	}

	/// Converts Doppler speed [-NI,NI] to unit circle.
	inline
	void mapDopplerSpeed(double d, double &x, double &y) const {
		d = scaleForward(d) * M_PI/NI;
		//std::cerr << d << '\n';
		x = cos(d);
		y = sin(d);
	}


protected:

	virtual void init();

};



// Proj4 proj could be part of this? No...
class CartesianODIM : public ODIM {

public:

	CartesianODIM(){
		init();
		object = "COMP";
	};

	CartesianODIM(const CartesianODIM & odim){
		init();
		updateFromMap(odim);
		//object = "COMP";
	};

	CartesianODIM(const drain::image::Image & image){
		// alert old?
		init();
		copyFrom(image);
	};

	template <class T>
	CartesianODIM(const std::map<std::string,T> & m){
		init();
		updateFromMap(m);  // updatemap?
	}

	inline
	void setGeometry(size_t cols, size_t rows){
		xsize = cols;
		ysize = rows;
	}


	/// WHERE
	std::string projdef;
	long xsize;
	long ysize;
	double xscale;
	double yscale;
	//drain::Rectangle<double> bboxD ?
	double LL_lat;
	double LL_lon;
	double UL_lat;
	double UL_lon;
	double LR_lat;
	double LR_lon;
	double UR_lat;
	double UR_lon;

	/// How cartesian data are processed, according to Table 12
	std::string camethod;

	/// Radar nodes (Table 9) which have contributed data to the composite, e.g. “’searl’, ’noosl’, ’sease’, ’fikor”’
	std::string nodes;

	/// This is needed for palette operations (of single-radar Cartesian products).
	//double NI;


	inline
	const drain::Rectangle<double> & getBoundingBoxD(){
		bboxD.xLowerLeft  = LL_lon;
		bboxD.yLowerLeft  = LL_lat;
		bboxD.xUpperRight = UR_lon;
		bboxD.yUpperRight = UR_lat;
		return bboxD;
	}

protected:

	virtual
	void init();

private:

	mutable drain::Rectangle<double> bboxD;

};

// Consider single-radar odim



}  // namespace rack


#endif

// Rack
