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
#include <algorithm>
//#include "drain/util/Options.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/Rectangle.h"
#include "drain/util/Time.h"
#include "drain/image/Geometry.h"

#include "hi5/Hi5.h"
#include "radar/Constants.h"

#include "EncodingODIM.h"

namespace rack {


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
 *  See also: LinearScaling (could be used as base class?)
 */


/// ODIM metadata (quantity, gain, offset, undetect, nodata, date, time)
class ODIM : public EncodingODIM { //, public  {

public:

	inline
	ODIM(group_t initialize = ODIMPathElem::ALL_LEVELS) : EncodingODIM(initialize){
		init(initialize);
	};

	inline
	ODIM(const ODIM & odim){
		initFromMap(odim);
	};

	inline
	ODIM(const drain::image::Image & image, const std::string & quantity="") : EncodingODIM(image) {
		initFromImage(image, quantity);
	};

	inline
	ODIM(drain::image::Image & image, const std::string & quantity="") : EncodingODIM(image) {
		initFromImage(image, quantity);
	};

	/*
	inline
	ODIM(const drain::image::Image & image, const std::string & quantity="") : EncodingODIM(image) {
		initFromImage(image, quantity);
	};
	*/

	/*
	inline
	ODIM(drain::image::Image & image, const std::string & quantity="") : geometryREF(image.geometry) {
		initFromImage(image, quantity);
	};
	*/

	inline
	~ODIM(){};

	//const drain::image::AreaGeometry geometry;

	drain::image::AreaGeometry area;

	/// Spatial resolution in metres
	/**
	 *  rscale for polar data
	 *  xscale, yscale for Cartesian data?
	 */
	drain::Point2D<double> resolution;

	//const drain::image::AreaGeometry & geometryTEST;

	/// Applied 8-digit date format, "%Y%m%d"
	static
	const std::string dateformat;

	/// Applied 6-digit date format, "%H%M%S"
	static
	const std::string timeformat;


	/// /what (obligatory)
	/// Data object, for example "PVOL" or "COMP"
	std::string object;
	std::string version;

	/// Nominal time, in dateformat
	std::string date;
	/// Nominal time, in timeformat
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

	// Elevation angles.
	// In ODIM, defined for Cartesian but used more generally - for polar products - in Rack
	std::vector<double> angles;


	// Number of images used in precipitation accumulation (lenient, not linked)
	long ACCnum;

	/// Sets number of columns (nbins) and number of rows (nrays). Does not change resolution.
	virtual inline
	void setGeometry(size_t cols, size_t rows){
		area.set(cols, rows);
	};


	virtual
	void setGeometry(const drain::image::AreaGeometry & g){
		setGeometry(g.getWidth(), g.getHeight());
	};

	virtual // ?
	const drain::image::AreaGeometry & getGeometry() const {
		return area;
	};

	/// Change geometry and adjust spatial resolution respectively.
	virtual
	void adjustGeometry(size_t cols, size_t rows);



	/// Updates object, quantity, product and time information.
	/*!
	 *  Fills empty values. Updates time variables.
	 */
	virtual
	void updateLenient(const ODIM & odim);

	/// Retrieves the stored time. Returns true if successful. // consider:  throws error if fail.
	inline
	bool getTime(drain::Time & t) const {
		return getTime(t, date, time);
	}

	/// Retrieves the start time. Returns true if successful. // consider:  throws error if fail.
	inline
	bool getStartTime(drain::Time & t) const {
		return getTime(t, startdate, starttime);
	}

	/// Retrieves the end time. Returns true if successful. // consider: throws error if fail.
	inline
	bool getEndTime(drain::Time & t) const {
		return getTime(t, enddate, endtime);
	}

	/// Sets \c date and \c time . Returns true if successful, throws error if fail.
	bool setTime(const drain::Time & t);

	/// Sets \c date and \c time given a string of  Returns true if successful, throws error if fail.
	bool setTime(const std::string & s);


	/// Returns recommended coordinate policy. Redefined in PolarODIM.
	virtual inline
	const drain::image::CoordinatePolicy & getCoordinatePolicy() const {
		//using namespace drain::image;
		static const drain::image::CoordinatePolicy policy(drain::image::CoordinatePolicy::LIMIT);
		return policy;
		//return rack::limit;
	}

	/// If nodata==undetect, set nodata=maxValue (hoping its not nodata...)
	/**
	 *  This oddity is needed because some manufactures do not distinguish between undetect and nodata (esp. with VRAD).
	 *
	 *  \param quantity - set only if quantity starts with this string. So "VRAD" covers "VRADH" and "VRADV", for example.
	 */
	bool distinguishNodata(const std::string & quantityPrefix = "");


	/// Write ODIM metadata to WHAT, WHERE and HOW groups.
	/**
	 *  \tparam G - group selector (number)
	 *  \tparam T - ODIM class
	 *
	 *  Examples of usage:
	 *  \code
	 * 	- ODIM::copyToH5<ODIMPathElem::ROOT>(odim, resources.inputHi5);
	 *	- ODIM::copyToH5<ODIMPathElem::DATASET>(odim, resources.inputHi5(dataSetPath));
	 *  - ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst);
	 *  \endcode
	 *
	 *  relevant for data level, eg. \c /dataset2, \c data4, and root.
	 */
	template <group_t G, class T>
	static inline
	void copyToH5(const T &odim, Hi5Tree & dst) {
		static const T odimLimited(G);
		static const std::list<std::string> & keys = odimLimited.getKeyList();
		//odim.copyTo(odimLimited.getKeyList(), dst);
		odim.copyTo(keys, dst);
	}

	template <group_t G, class T>
	static inline
	void copyToH5(const T &odim, const Hi5Tree & dst) {
		//static T odimLimited(G);
		//odim.copyTo(odimLimited.getKeyList(), dst);
	}




	// Maximum Nyquist velocity, mutable because may be updated with PolarODIM::getNyquist()
	mutable
	double NI;



protected:

	/// Retrieves the stored time. Returns true if successful, throws error if fail.
	bool getTime(drain::Time & t, const std::string &dateStr, const std::string &timeStr) const;


	///
	void copyTo(const std::list<std::string> & keys, Hi5Tree & dst) const;


	template <class T>
	inline
	void initFromMap(const std::map<std::string,T> & m){
		init(ODIMPathElem::ALL_LEVELS);
		updateFromMap(m);
	}

	virtual inline
	void initFromImage(const drain::image::Image & img){  // =""
		init(ODIMPathElem::ALL_LEVELS);
		this->quantity = img.getProperties().get("what:quantity", "");
		copyFrom(img);
	}

	// deprec
	virtual inline
	void initFromImage(const drain::image::Image & img, const std::string & quantity){  // =""
		drain::Logger mout(__FILE__, __FUNCTION__);
		init(ODIMPathElem::ALL_LEVELS);
		this->quantity = quantity;
		copyFrom(img);
		//mout.warn("guantity finally=(" , this->quantity , '<' , quantity , ")" );
		//mout.warn(*this );
	}

private:


	virtual // must
	void init(group_t initialize =ODIMPathElem::ALL_LEVELS);



};


inline
std::ostream & operator<<(std::ostream &ostr, const ODIM & odim){
	odim.drain::SmartMap<drain::Referencer>::toStream(ostr);
	return ostr;
}

}  // namespace rack


#endif

// Rack
 // REP // REP // REP
