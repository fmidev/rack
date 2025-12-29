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
// Thanks to: Mikael Kilpeläinen

#ifndef DATA_H_
#define DATA_H_

#include <data/QuantitySelector.h>
#include <set>
#include <list>
#include <map>

#include <drain/RegExp.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/StringMatcherList.h>

#include "DataSelector.h"
#include "DataTools.h"
#include "PolarODIM.h"
#include "CartesianODIM.h"
#include "VerticalODIM.h"


using namespace drain::image;

namespace rack {

/**

 Consider that const Hi5Tree src or Hi5Tree dst contain following structure:

 \code
 /dataset1/data1
 /dataset1/data1/quality1/data
 /dataset1/data2
 /dataset1/data3
 /dataset1/dataN...
 /dataset1/quality1/data
 /dataset1/quality2/data
 /dataset2/
 ...
 \endcode

  The classes in this section provide solutions for handling dataset-level and data-level information.


 */


/// Container that couples together a tree structure and a data array (drain::image::Image), according to source type (const or non-const) .
/**
 *  The template parameters are expected to be exactly the following, all of them const or non-const:
 *  \tparam T  - HDF5 tree, as Hi5Tree
 *  \tparam TI - tree iterator type
 *  \tparam D  - data array (drain::image::Image)
 */
template <typename T, typename TI, typename D>
struct DataType {
	typedef T  tree_t;
	typedef TI tree_iter_t;
	typedef D  image_t;
};


/*
template <class T, size_t N>
struct TypeName<UniTuple<T,N> > {

    static const std::string & str(){
		static std::string name = drain::StringBuilder<>("UniTuple<", drain::TypeName<T>::str(), ',', N, ">");
        return name;
    }

};
*/

/// Read-only data type
/**
 *  \tparam M - meta data using OPERA data information model (ODIM)
 */
template <typename M = PolarODIM const>
struct SrcType : public DataType<Hi5Tree const, Hi5Tree::const_iterator, Image const> {
	typedef M odim_t;
};


/// Writable data type
/**
 *  \tparam M - meta data using OPERA data information model (ODIM)
 */
template <typename M = PolarODIM>
struct DstType : public DataType<Hi5Tree, Hi5Tree::iterator, Image> {
	typedef M odim_t;
};

/// Read-only data (supports top-level ODIM properties only)
typedef SrcType<ODIM const> BasicSrc;

/// Read-only data in polar coordinates
typedef SrcType<PolarODIM const> PolarSrc;

/// Read-only data in Cartesian coordinates
typedef SrcType<CartesianODIM const> CartesianSrc;


typedef DstType<ODIM> BasicDst;

/// Writable data in polar coordinates
typedef DstType<PolarODIM> PolarDst;

/// Writable data in Cartesian coordinates
typedef DstType<CartesianODIM> CartesianDst;

/// Writable data in vertical coordinates, with (pseudo) RHI metadata
typedef DstType<RhiODIM> RhiDst;

/// Writable data in vertical coordinates, with profile metadata
typedef DstType<VerticalProfileODIM> VprDst;



/// Base class for all kinds of radar data
/**
 *  Provides a layer on top of the raw HDF5 trees, so that data and attributes
 *  can be accessed more directly using ODIM HDF5 conventions.
 *
 *  \tparam DT - data type: SrcType<T> or DstType<T> with ODIM template T = [PolarODIM|CartesianODIM|...]
 */
template <typename DT>
class TreeWrapper {
public:

	typedef DT datatype_t;
	typedef typename DT::tree_t tree_t;
	typedef typename DT::tree_iter_t tree_iter_t;

	inline
	const drain::VariableMap & getWhat() const {
		return getAttr<ODIMPathElem::WHAT>();
	}

	inline
	drain::VariableMap & getWhat() {
		return getAttr<ODIMPathElem::WHAT>();
	}

	inline
	const drain::VariableMap & getWhere() const {
		return getAttr<ODIMPathElem::WHERE>();
	}

	inline
	drain::VariableMap & getWhere() {
		return getAttr<ODIMPathElem::WHERE>();
	}


	inline
	const drain::VariableMap & getHow() const {
		return getAttr<ODIMPathElem::HOW>();
	}

	inline
	drain::VariableMap & getHow() { // ODIMPathElem(ODIMPathElem::HOW)
		return getAttr<ODIMPathElem::HOW>();
	}

	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	void setExcluded(bool exclude = true){
		this->tree.data.exclude = exclude;
	};

	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	bool isExcluded() const {
		return this->tree.data.exclude;
	};

	//typename DT::odim_t odim;

	virtual inline
	const tree_t & getTree() const { return this->tree; };

	// expected public, at least by DetectorOp::storeDebugData()
	virtual inline
	tree_t & getTree(){ return this->tree; } ;


protected:

	/// General HDF5 data structure
	tree_t & tree;

	TreeWrapper(tree_t & tree) : tree(tree){
	};

	//TreeWrapper(tree_t & tree, const odim_t & odim) tree(tree), odim(odim) {};
	TreeWrapper(const TreeWrapper & d) : tree(d.tree){}; //, image(d.data), odim(d.odim) {};


	~TreeWrapper(){
		/*
		drain::Logger mout("TreeWrapper", __FUNCTION__);
		if (this->tree.data.exclude){
			mout.note("deleting (children only?)" );
			this->tree.clear();
		}
		*/
	};


	template <ODIMPathElem::group_t G>
	const drain::VariableMap & getAttr() const {
		return this->tree[ODIMPathElem(G)].data.attributes;
	}

	template <ODIMPathElem::group_t G>
	drain::VariableMap & getAttr(){
		return this->tree[ODIMPathElem(G)].data.attributes;
	}


};

// In future, could be the interface for wrapping the whole structure?

/// Experimental structure, designed only for accessing root level metadata.
/**
 *  Not extensively used...
 *
 *
 *
 */
template <typename DT>
class RootData : public TreeWrapper<DT> {

public:

	RootData(typename DT::tree_t & tree) : TreeWrapper<DT>(tree) { // This could be good: odim(ODIMPathElem::ROOT) ... but failed with current design
		// experimental
		this->odim.copyFrom(tree.data.image); // <- works (only) if updateInternalAttributes() has been called?
	};

	virtual inline
	~RootData(){

		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.experimental<LOG_DEBUG>("invoking ODIM::updateH5AttributeGroups<ODIMPathElem::ROOT>()");
		mout.experimental<LOG_DEBUG+1>("root odim: ", this->odim);

		ODIM::updateH5AttributeGroups<ODIMPathElem::ROOT>(this->odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // overrides anything?
	};

	// Metadata structure
	typename DT::odim_t odim;

};


/// Essential class for storing radar data.
/**
 *  Combines 2D image data and metadata.
 *
 *  For metadata, uses ODIM variables, hence replacing native
 *
 *  \tparam DT - data type: SrcType<T> or DstType<T> with ODIM template T = [PolarODIM|CartesianODIM|...]
 *     (PolarSrc, PolarDst, CartesianSrc, CartesianDst, ...)
 */
template <typename DT>
class PlainData : public TreeWrapper<DT> {
public:

	typedef typename DT::tree_t tree_t;
	typedef typename DT::image_t image_t;
	typedef typename DT::odim_t odim_t;

	// NEW
	inline
	PlainData(typename DT::tree_t & tree) :
		TreeWrapper<DT>(tree),
		data(tree[ODIMPathElem::ARRAY].data.image), // "data"
		odim(data, data.properties.get("what:quantity",""))
	{
		//data.setScaling(odim.scaling.scale, odim.scaling.offset);
	}

	/// Constructor referring to HDF5 structure
	PlainData(typename DT::tree_t & tree, const std::string & quantity) :
		TreeWrapper<DT>(tree),
		data(tree[ODIMPathElem::ARRAY].data.image), // "data"
		odim(data, quantity) // reads data.properties?
	{
				//data.setScaling(odim.scaling.scale, odim.scaling.offset);
	}

	/// Copy constructor, also for referencing non-const as const.
	/**
	 *   Compiler returns error if odim types ar incompatible.
	PlainData(const PlainData<DT> & d) : TreeWrapper<DT>(d.getTree()),
		data(this->tree["data"].data.image),
		odim(data)  // NEW
	{
	}
	 */

	template <typename DT2>
	PlainData(const PlainData<DT2> & d) : TreeWrapper<DT>(d.getTree()),
		data(this->tree[ODIMPathElem::ARRAY].data.image), // "data"
		// odim(d.odim) // OLD
		odim(data)  // NEW
	{
		//odim.updateFromMap(d.odim); // NEW
		//odim.updateFromCastableMap(d.odim); // NEW
		//data.setScaling(odim.scaling.scale, odim.scaling.offset);
	}

	inline
	~PlainData(){
		//drain::Logger mout("PlainData", __FUNCTION__);
		//mout.debug2("calling updateTree2, odim: " , odim );
		updateTree2();
	}

	/// Saves type and sets the type of the actual data array as well.
	template <class T>
	inline
	void setEncoding(const T & type, const std::string & values = ""){
		//odim.type = drain::Type::getTypeChar(type);
		this->odim.type = drain::Type::getTypeChar(drain::Type(type));
		this->odim.setTypeDefaults(type, values);
		data.setType(type);
		// TODO: data.setScaling(odim.scaling); ??
	}

	/// New, experimental.
	inline
	void copyEncoding(const EncodingODIM & odim){
		this->odim.importMap(odim);
		data.setType(this->odim.type);
		data.setScaling(this->odim.scaling); // needed?
	}


	template <class DT2>
	inline
	void copyEncoding(const PlainData<DT2> & srcData){
		this->odim.importMap(srcData.odim);
		data.setType(this->odim.type);
		data.setScaling(this->odim.scaling); // needed?
	}

	inline
	void setPhysicalRange(double min, double max){
		//data.setPhysicalScale(min, max);
		data.setPhysicalRange(min, max, true);
		// data.setOptimalScale();
		this->odim.scaling.assignSequence(data.getScaling());
		// odim.scaling.scale   = data.getScaling().scale; // needed?
		// odim.scaling.offset = data.getScaling().offset; // needed?
	}



	/// Sets dimensions of data array and metadata.
	inline
	void setGeometry(size_t cols, size_t rows){
		this->odim.setGeometry(cols, rows);
		data.setGeometry(cols, rows);
	}

	/// Sets dimensions of data array and metadata.
	inline
	void setGeometry(const drain::image::AreaGeometry & geometry){
		this->odim.setGeometry(geometry.getWidth(), geometry.getHeight());
		data.setGeometry(geometry);
	}

	/// Copy dimensions of data array and resolution (rscale or xscale,yscale)
	/**
	 *   Although templated, makes sense only across the Src and Dst types of same ODIM class.
	 */
	inline
	void setGeometry(const odim_t & odim){
		this->odim.setGeometry(odim.area);
		data.setGeometry(odim.area);
		// Note:
		this->odim.resolution = odim.resolution;
	}

	/// Copy dimensions of data array and resolution (rscale or xscale,yscale)
	/**
	 *   Although templated, makes sense only across the Src and Dst types of same ODIM class.
	 */
	template <class DT2>
	inline
	void copyGeometry(const PlainData<DT2> & srcData){
		setGeometry(srcData.odim);
		/*
		this->odim.setGeometry(srcData.odim.area);
		data.setGeometry(srcData.odim.area);
		// Note:
		this->odim.resolution = srcData.odim.resolution;
		*/
	}

	/// Calls setEncoding() and setGeometry().
	template <class T>
	inline
	void initialize(const T & type, size_t cols, size_t rows){
		setEncoding(type);
		setGeometry(cols, rows);
	}

	template <class T>
	inline
	void initialize(const T & type, const drain::image::AreaGeometry & geometry){
		setEncoding(type);
		setGeometry(geometry);
	}

	// NEW policy.
	/// Terminal step in initialisation: set actual data storage type and resize.
	inline
	void initializeBest(){
		setEncoding(odim.type);
		setGeometry(odim.area);
	}

	/// Set storage type and other properties.
	template <class ...T>
	inline
	void initializeBest(const std::type_info & type, const T & ...args){
		odim.setType(type);
		initializeBest(args...);
	}

	/// Set storage type and other properties.
	template <class ...T>
	inline
	void initializeBest(const std::string & type, const T & ...args){
		odim.setType(type);
		initializeBest(args...);
	}

	/// Set width and height of the data array, and set other properties.
	template <class ...T>
	inline
	void initializeBest(const drain::image::AreaGeometry & geometry, const T & ...args){
		odim.area.set(geometry);
		initializeBest(args...);
	}

	/// Set encoding and other properties.
	template <class ...T>
	inline
	void initializeBest(const EncodingODIM & encoding, const T & ...args){
		odim.updateFromMap(encoding); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
		odim.scaling.physRange.set(encoding.scaling.physRange);
		initializeBest(args...);
	}


	// inline
	void setNyquist(double nyquistSpeed){
		odim.NI = nyquistSpeed;
		odim.setRange(-odim.NI, +odim.NI);
		data.setScaling(odim.scaling);
	}


	// Data array
	image_t & data;

	// Metadata structure
	odim_t odim; // 2023/01 considered ...raising to TreeContainer

	//drain::Legend legend;

	// Possibly this should be somewhere else? (Too specific here?)
	/// For this data, creates an on-off quality data.
	void createSimpleQualityData(drain::image::Image & qualityImage, double dataQuality=1.0, double undetectQuality=0.5, double nodataQuality=0.0) const;

	// Possibly this should be somewhere else? (Too specific here?)
	/// For this data, creates an on-off quality data.
	inline
	void createSimpleQualityData(PlainData<DT> & qualityData, double dataQuality=1.0, double undetectQuality=0.5, double nodataQuality=0.0) const { //, double dataQuality=1.0, double nodataQuality=0.0) const {
		qualityData.setEncoding(typeid(unsigned char));
		createSimpleQualityData(qualityData.data, dataQuality, undetectQuality, nodataQuality);
		qualityData.odim.scaling.set(qualityData.data.getScaling());
		//qualityData.odim.scaling.scale   = qualityData.data.getScaling().scale;
		//qualityData.odim.scaling.offset = qualityData.data.getScaling().offset;
	}

	/// TODO: consider this to destructor
	inline
	void updateTree2(){
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATA>(this->odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // Needed? The subtree is small... But for quality field perhaps.
	}


protected:


};

/**
 *  \tparam DT - data type (PolarSrc, PolarDst, CartesianSrc, CartesianDst, ...)
 */
template <typename DT>  // PlainData<DT> & quality
void PlainData<DT>::createSimpleQualityData(drain::image::Image & quality, double dataQuality, double undetectQuality, double nodataQuality) const {

	quality.setPhysicalRange(0.0, 1.0, true);

	const drain::ValueScaling & scaling = quality.getScaling();

	const bool DATA     = !std::isnan(dataQuality);
	const bool UNDETECT = !std::isnan(undetectQuality);
	const bool NODATA   = !std::isnan(nodataQuality);

	// Default ie. unset values are non_signaling_NAN's, but maybe more elegant to skip calculations:
	const double dataCode     = DATA     ? scaling.inv(dataQuality)     : 0.0;
	const double undetectCode = UNDETECT ? scaling.inv(undetectQuality) : 0.0;
	const double nodataCode   = NODATA   ? scaling.inv(nodataQuality)   : 0.0;

	quality.setGeometry(data.getWidth(), data.getHeight());

	Image::iterator  it = data.begin();
	Image::iterator wit = quality.begin();
	while (it != data.end()){
		//if ((*it != odim.nodata) && (*it != odim.undetect))
		if (UNDETECT && (*it == this->odim.undetect))
			*wit = undetectCode;
		else if (NODATA && (*it == this->odim.nodata))
			*wit = nodataCode;
		else if (DATA)
			*wit = dataCode;
		++it;
		++wit;
	}

	/*
	const double d  = scaling.inv(dataQuality);
	const double nd = scaling.inv(nodataQuality);
	const double un = scaling.inv(undetectQuality);

	quality.setGeometry(data.getWidth(), data.getHeight());

	Image::iterator  it = data.begin();
	Image::iterator wit = quality.begin();
	while (it != data.end()){
		//if ((*it != odim.nodata) && (*it != odim.undetect))
		if (*it == odim.nodata)
			*wit = nd;
		else if (*it == odim.undetect)
			*wit = un;
		else
			*wit = d;
		++it;
		++wit;
	}
	*/

}



/**
 *  \tparam DT - data type (PolarSrc, PolarDst, CartesianSrc, CartesianDst, ...)
 */
template <typename DT>
inline
std::ostream & operator<<(std::ostream & ostr, const PlainData<DT> & d){
	ostr << d.data << ", ODIM:\t ";
	ostr << d.odim << '\n';
	ostr << "props: " << d.data.properties << '\n';
	return ostr;
}


/// A map of "data type" group_t (DATA or QUALITY) where the data can be retrieved using quantity keys (strings).
/**

	For example, dataGroup["DBZH"] may return data located at "data2"
	and qualityGroup["QIND"] may return data at "quality1".

	Also based on TreeWrapper, so has a \c tree inside...

    \tparam DT - data object type: PlainData<...> or Data<...>
    \tparam G  - the path element of children: ODIMPathElem:: DATASET, DATA, or QUALITY

   Applications: see SweepSrc and ProductDst below.
 */
template <class DT, ODIMPathElem::group_t G>
class DataGroup : public TreeWrapper<typename DT::datatype_t>, public std::map<std::string, DT > { // typename T::datatype_t
public:

	typedef DataGroup<DT,G> datagroup_t; // this type
	typedef DT data_t;
	typedef typename DT::datatype_t datatype_t;
	typedef std::map<std::string, DT > map_t;

	// Experimental. Mainly for geometry (width, height) - but also for date+time or elangle?
	/*
	typedef typename DT::odim_t odim_t;
	odim_t baseODIM;


	inline
	void setGeometry(size_t width, size_t height){
		baseODIM.setGeometry(width, height);
	}

	inline
	void setGeometry(const drain::image::AreaGeometry & geometry){
		baseODIM.setGeometry(geometry);
	}
	*/
	//typename DT::odim_t odim;// 2023/01 experimental


	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.

	// DataGroup(typename DT::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
	// TreeWrapper<typename DT::datatype_t>(tree), odim(tree.data.image) {
	DataGroup(typename DT::tree_t & tree, const QuantitySelector & slct = QuantitySelector()) :
		TreeWrapper<typename DT::datatype_t>(tree) { //, baseODIM(ODIMPathElem::DATASET) {
		init(tree, *this, slct);
	}
	//init(tree, *this, quantityRegExp);

	/*
	DataGroup(typename DT::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		TreeWrapper<typename DT::datatype_t>(tree) {
		init(tree, *this, quantityRegExp);
	}
	*/

	// TreeWrapper<typename DT::datatype_t>(src.tree), odim(src.tree.data.image) {

	DataGroup(const datagroup_t & src) :
		TreeWrapper<typename DT::datatype_t>(src.tree) { // , baseODIM(ODIMPathElem::DATASET){
		adapt(src, *this);  // ALERT: includes all the quantities, even thoug src contained only some of them
	}


	virtual
	~DataGroup(){
		/*
		drain::Logger mout("DataGroup<" + ODIMPathElem::getKey(G)+">", __FUNCTION__);
		switch (this->size()) {
		case 0:
			mout.debug3("no data<n> groups" );
			break;
		default:
			mout.info("several Data groups, using: " , this->begin()->first );
			// no break;
		case 1:
			mout.debug("updating from 1st data: " , this->begin()->first );
			updateTree3(this->getFirstData().odim); // tree
		}
		*/
	};


	bool has(const std::string & quantity) const {
		return (this->find(quantity) != this->end());
	}

	inline
	const data_t & getData(const char *quantity) const {
		return getData(std::string(quantity));
		//return getData(quantity.c_str());
	}


	const data_t & getData(const std::string & quantity) const {

		//drain::Logger mout(__FILE__, __FUNCTION__); //
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}");

		typename datagroup_t::const_iterator it = this->find(quantity);

		if (it != this->end()){
			mout.debug3('[' , quantity , "]\t = " , it->first );
			return it->second;
		}
		else {
			mout.debug('[' , quantity , "] not found, returning empty"  );
			return getEmpty();
		}
	}

	inline
	data_t & getData(const char *quantity) {
		return getData(std::string(quantity));
		//return getData(quantity.c_str());
	}


	data_t & getData(const std::string & quantity) {

		//drain::Logger mout(__FILE__, __FUNCTION__); //REPL "DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}");

		//mout.warn("non-const " );
		typename datagroup_t::iterator it;
		#pragma omp critical  //(h5insert)
		{
			it = this->find(quantity);
			if (it != this->end()){
				mout.debug3("found " , it->first );
			}
			else {
				//mout.note("not found, creating new data array" );
				ODIMPathElem child(G);
				ODIMPathTools::getNextChild(this->tree, child);
				mout.debug3("add: " , child , " [" , quantity , ']' );
				it = this->insert(this->begin(), typename map_t::value_type(quantity, DT(this->getTree()[child], quantity)));  // WAS [path]
				// it->second.setGeometry(baseODIM.getGeometry());
			}
		}
		return it->second;

	}

	//const data_t& getData(const drain::RegExp & regExp) const {
	const data_t& getData(const QuantitySelector & slct) const {

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G)+"(RegExp) {const}", __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "(KeySelector): ", "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		// NEW
		for (const drain::StringMatcher & m: slct.getList()){
			for (const auto & entry: *this){
				if (m.test(entry.first)){
					mout.debug("quantity " , entry.first , " matches " , slct);
					return entry.second;
				}
			}
		}

		/*  OLD
		for (typename datagroup_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (slct.testQuantity(it->first)){
				mout.debug("quantity " , it->first , " matches " , slct);
				return it->second;
			}
		}
		*/

		mout.note("no quantity matched with " , slct);

		return getEmpty();

	}

	/// Creates (or overrides) data array for \c quantity and scales it
	/**
	 *	\param quantity -
	 *	\param templateQuantity - predefined quantity the scaling of which is used in initialisation
	 *	\param encodingParams (optional) - parameters overriding those of template quantity
	 */
	data_t & create(const std::string & quantity) { // , const std::string & templateQuantity, const std::string & encodingParams) {
		data_t & d = getData(quantity);
		d.setGeometry(0, 0); // in case existed already
		//getQuantityMap().setQuantityDefaults(d, templateQuantity, encodingParams);
		return d;
	}


	data_t & getFirstData() {

		for (auto & entry: *this){
			if (!entry.second.isExcluded()){
				return entry.second;
			}
		}

		drain::Logger(__FILE__, __LINE__, "DataGroup{" + ODIMPathElem::getKey(G)+"}:", __FUNCTION__).error("no data");
		//mout.error("no data" );
		return this->getData(""); // empty?

		/*
		const typename datagroup_t::iterator it = this->begin();

		if (it == this->end()){
			//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
			drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

			mout.error("no data" );
			return this->getData("");
		}
		else
			return it->second;
		*/
	}

	const data_t & getFirstData() const {

		for (const auto & entry: *this){
			if (!entry.second.isExcluded()){
				return entry.second;
			}
		}

		drain::Logger(__FILE__, __LINE__, "DataGroup{" + ODIMPathElem::getKey(G)+"}:", __FUNCTION__).error("no data");
		return this->getData(""); // empty?


		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G) + " {const}", __FUNCTION__);
		/*
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		//mout.warn("const" );

		typename datagroup_t::const_iterator it = this->begin();

		if (it != this->end()){
			mout.debug2("found: " , it->first );
			return it->second;
		}
		else {
			mout.note("not found, returning empty"  );
			return getEmpty();
		}
		*/

	}


	// experimental
	data_t & getLastData() {

		const typename datagroup_t::reverse_iterator it = this->rend();

		if (it == this->rbegin()){
			drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");
			mout.error("no data" );
			return this->getData("");
		}
		else
			return it->second;

	}

	// experimental
	const data_t & getLastData() const {

		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}-const");

		//mout.warn("const" );

		typename datagroup_t::const_reverse_iterator it = this->rend();

		if (it != this->rbegin()){
			mout.debug2("found: " , it->first );
			return it->second;
		}
		else {
			mout.note("not found, returning empty"  );
			return getEmpty();
		}


	}


protected:

	static
	const data_t & getEmpty() {
		static typename DT::tree_t t;
		static data_t empty(t);
		return empty;
	}

	/// Given a \c dataset h5 subtree, like tree["dataset3"], collects all (or desired) quantities to a data object.
	/**
	 *  \param t   - target data tree
	 *	\param dst - odim wrapper for the data tree
	 * 	\param quantityRegExp - optional quantity filter, if only a subset is desired.
	 */
	static
	typename DT::tree_t & init(typename DT::tree_t & t, datagroup_t & dst, const QuantitySelector & slct = QuantitySelector()){
	// typename DT::tree_t & init(typename DT::tree_t & t, datagroup_t & dst, const drain::RegExp & quantityRegExp = drain::RegExp()){

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		//const bool USE_REGEXP = quantityRegExp.isSet();
		const bool USE_SELECTOR = slct.isSet();

		// Number of potential groups for debug note at end
		unsigned short counter = 0;

		mout.debug3("collecting data items, selector='", slct, "'");
		//if (USE_REGEXP)
		//	mout << ", regExp=" << quantityRegExp.toStr();
		//mout << mout.endl;

		// #pragma omp critical //(h5insert2)
		{
			// add UKMO

			const std::string datasetQuantity = t[ODIMPathElem::WHAT].data.attributes.get("quantity", "");

			//for (typename DT::tree_iter_t it=t.begin(); it!=t.end(); ++it){
			for (auto & entry: t){

				/// Accept groups of type G only
				if (! (entry.first.is(G))){
					//mout.warn("skip '" , it->first , "' \t group != " , G );
					continue;
				}

				//const std::string dataQuantity = it->second["what"].data.attributes["quantity"];
				//const std::string dataQuantity = it->second[ODIMPathElem::WHAT].data.attributes["quantity"];
				const std::string dataQuantity = entry.second[ODIMPathElem::WHAT].data.attributes.get("quantity", ""); // otherways comes "null"

				const std::string & quantity = !dataQuantity.empty() ? dataQuantity : datasetQuantity;

				if (USE_SELECTOR){
					++counter; // candidate count
					if (!slct.test(quantity)){
					//if (!quantityRegExp.test(quantity)){
						//if (it->second.hasChild("quality1"))
						//	mout.warn(it->first , "...rejecting, but has quality?" );
						mout.debug3("rejected '" , entry.first , "' [" , quantity , "] !~" , slct ); // quantityRegExp.toStr()
						continue;
					}
				}

				/*
				if (USE_REGEXP){
					++counter; // candidate count
					if (!quantityRegExp.test(quantity)){
						//if (it->second.hasChild("quality1"))
						//	mout.warn(it->first , "...rejecting, but has quality?" );
						mout.debug3("rejected '" , entry.first , "' [" , quantity , "] !~" , quantityRegExp.toStr() );
						continue;
					}
				}
				*/

				mout.accept<LOG_DEBUG+2>("accept '", entry.first, "' [", quantity, ']' );

				/*
				mout.warn(entry.first, " 1st -> ", entry.second[ODIMPathElem::ARRAY].data.image);
				mout.fail("type", drain::TypeName<DT>::get());
				DT(entry.second, quantity);
				mout.warn(entry.first, " 1bst-> ", entry.second[ODIMPathElem::ARRAY].data.image);
				*/

				if (quantity.empty()){
					//drain::Logger mout("DataSet", __FUNCTION__);
					mout.info("quantities dataset:'", datasetQuantity, "', data:'", dataQuantity, "'");
					mout.warn("undefined quantity in ", entry.first, ", using key=", entry.first );
					// Assign by path component "data3"
					dst.insert(typename map_t::value_type(entry.first, DT(entry.second, entry.first)));
					//associate(dst, it->first, it->second);
				}
				else {
					if (dst.find(quantity) != dst.end()){ // already created
						//drain::Logger mout("DataSet", __FUNCTION__);
						mout.warn("quantity '" , quantity , "' replaced same quantity at " , entry.first );
					}
					dst.insert(typename map_t::value_type(quantity, DT(entry.second, quantity)));
					//typename datagroup_t::reverse_iterator rit = dst.rend();
					//mout.warn("last '" , "' [" , quantity , '=' , rit->first , ']' , rit->second );
					//dst[quantity] = T(it->second);
					// mout.warn(entry.first, " 2nd -> ", entry.second[ODIMPathElem::ARRAY].data.image);
				}
			}
		} // end pragma

		if (USE_SELECTOR)
			mout.debug3("matched " , dst.size() , "(out of " , counter , ") data items with selector: " , slct , '/' );
			//mout.debug3("matched " , dst.size() , "(out of " , counter , ") data items with RegExp=/" , quantityRegExp.toStr() , '/' );
		else
			mout.debug3("collected " , dst.size() , " data items" );

		return t;
	};


	static
	//typename D::tree_t & adapt(typename D::tree_t & t, datagroup_t & dst, const datagroup_t & src){
	typename DT::tree_t & adapt(const datagroup_t & src, datagroup_t & dst){

		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		// drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		// drain::Logger mout(__FILE__, __FUNCTION__);

		if (src.empty()){
			mout.debug3("src empty" );
			return src.tree;
		}
		else {
			for (const auto & entry: src){
				dst.insert(entry);
			}
		}

		/*
		for (typename datagroup_t::const_iterator it=src.begin(); it!=src.end(); ++it){
			//dst.insert(typename map_t::value_type(it->first, D(it->second, it->first)));
			//dst.insert(typename map_t::value_type(it->first, D(it->second)));
			dst.insert(typename map_t::value_type(it->first, it->second));
		}
		*/
		mout.debug3("adapted ", dst.size(), " data items; ", src.begin()->first, "...");

		//return t
		return src.tree;
	};


};

template <class DT, ODIMPathElem::group_t G>
std::ostream & operator<<(std::ostream & ostr, const DataGroup<DT,G> & d){
	// ostr << "-- dataGroup ";
	char separator = 0;
	drain::image::Geometry g;
	for (typename DataGroup<DT,G>::const_iterator it = d.begin(); it != d.end(); ++it){
		if (separator)
			ostr << separator;
		else {
			separator = ',';
			g.setGeometry(it->second.data.getGeometry());
		}
		ostr << it->first << '[' << drain::Type::getTypeChar(it->second.data.getType()) << ']';
	}
	ostr << " ("<< g << ")";
	/*
	ostr << d.data << ", ODIM:\t ";
	ostr << d.odim << '\n';
	ostr << "props: " << d.data.properties << '\n';
	*/
	return ostr;
}


/// Base class providing quality support for Data<DT> and DataSet<DT>
/**
 *  Essentially, an instance of QualityDataSupport is something which has quality data as members.
 *  This data is stored in quality group, which is a map of quality fields.
 *
 */
template <typename DT>
class QualityDataSupport {

public:

	typedef PlainData<DT> plaindata_t;

	typedef DataGroup<plaindata_t,ODIMPathElem::QUALITY> qualitygroup_t;


	inline
	QualityDataSupport(typename plaindata_t::tree_t & tree) : quality(tree) {};

	inline
	QualityDataSupport(const QualityDataSupport<DT> & qds) : quality(qds.quality) {};

	virtual inline
	~QualityDataSupport(){};

	/// Finds associated quality data - maybe empty and unscaled.
	/*
	 *  \param quantity - quality quantity, "QIND" by default.
	 *  \return - \c data[i]/quality[j] for which \c quantity=[quantity]
	 *
	 */
	inline
	const plaindata_t & getQualityData(const std::string & quantity = "QIND") const {
		return this->quality.getData(quantity);
	}

	/// Finds associated quality data - maybe empty and unscaled.
	/*
	 *  \param quantity - quality quantity, "QIND" by default.
	 *  \return - \c data[i]/quality[j] for which \c quantity=[quantity]
	 *
	 */
	/*
	inline
	const plaindata_t & getQualityData(const drain::RegExp & quantityRE) const {
		return this->quality.getData(quantityRE);
	}
	*/


	/// Finds associated quality data - maybe empty and unscaled.
	/*
	 *  \param quantity - quality quantity, "QIND" by default.
	 *  \return - \c data[i]/quality[j] for which \c quantity=[quantity]
	 *
	 */
	inline
	plaindata_t & getQualityData(const std::string & quantity = "QIND") {
		return this->quality.getData(quantity);
	}


	inline
	bool hasQuality() const {
		return !this->quality.empty();
	}

	inline
	bool hasQuality(const std::string & quantity) const {
		return this->quality.find(quantity) != this->quality.end();
	}

	inline
	const qualitygroup_t & getQuality() const {
		return this->quality;
	}

	inline
	qualitygroup_t & getQuality(){
		return this->quality;
	}

	/* Well, needs quantity, primarily. So best place perhaps not here.
	static
	void createSimpleQualityData(PlainData<DT> & data, drain::image::Image & qualityImage, double dataQuality=1.0, double undetectQuality=0.5, double nodataQuality=0.0) const;

	static inline
	void createSimpleQualityData(PlainData<DT> & data, PlainData<DT> & qualityData, double dataQuality=1.0, double undetectQuality=0.5, double nodataQuality=0.0) const { //, double dataQuality=1.0, double nodataQuality=0.0) const {
		qualityData.setEncoding(typeid(unsigned char));
		createSimpleQualityData(qualityData.data, dataQuality, undetectQuality, nodataQuality);
		qualityData.odim.scaling.set(qualityData.data.getScaling());
		//qualityData.odim.scaling.offset =  qualityData.data.getScaling().offset;
	}
	*/

protected:

	qualitygroup_t quality;

};



/*
 *  Data<PolarSrc> will return
 *
 *  Quality field:
 *  - NONE: reference to dummy.
 *  - LAST: reference to last existing quality[1].
 *  - NEXT: reference to new quality[N+1], where N is the last
 */

/// Data structure consisting of plain data and an optional quality data.
/**
 *
 *  \tparam DT - data type (PolarSrc, PolarDst, CartesianSrc, CartesianDst)
 *
 */
template <typename DT>
class Data : public PlainData<DT>, public QualityDataSupport<DT> {
public:

	typedef PlainData<DT> plaindata_t;

	Data(typename DT::tree_t & tree) : PlainData<DT>(tree), QualityDataSupport<DT>(tree) {
	}

	Data(typename DT::tree_t & tree, const std::string & quantity) : PlainData<DT>(tree, quantity), QualityDataSupport<DT>(tree) {
	}

	Data(const Data<DT> & d) : PlainData<DT>(d), QualityDataSupport<DT>(d) {  // was: d.tree,    d.tree
	}

	// Data(const Hi5Tree & src, const std::string & quantity = "^DBZH$");
	virtual ~Data(){};

	// Experimental
	void swap(Data<DT> &d){ // TODO: also for plaindata?

		drain::Logger mout("Data<DT>", __FUNCTION__);
		mout.experimental("Swapping...");
		this->tree.swap(d.tree);

		typename DT::odim_t odim;
		odim.updateFromMap(this->odim);
		this->odim.updateFromMap(d.odim);
		d.odim.updateFromMap(odim);

		this->updateTree2();
		d.updateTree2();
	}

protected:

};





template <typename DT>
inline
std::ostream & operator<<(std::ostream & ostr, const Data<DT> & d){
	ostr << d.data << ", ODIM:\t ";  // .getGeometry()
	ostr << d.odim; // << '\n';
	//ostr << d.data.properties << '\n';
	if (d.hasQuality()){
		//ostr << " (has quality field)";
		ostr << "+q(" << d.getQuality() << ')';
		/*
		const PlainData<T,D,M> & q = d.getQuality();
		ostr << '\t' << q.data.getGeometry() << '\t';
		ostr << q.odim << '\n';
		ostr << '\t' << q.data.properties << '\n';
		 */
	}
	ostr << '\n';
	return ostr;
}


/// A map of radar data, indexed by quantity code (DBZH, VRAD, etc).
/**
    \tparam TD  - datatype: PolarSrc, CartesianSrc or

   See SweepSrc and ProductDst below.
 */
template <typename DT>
class DataSet : public DataGroup<Data<DT>,ODIMPathElem::DATA>, public QualityDataSupport<DT> { // typename T::data_t
public:

	typedef Data<DT> data_t;
	typedef PlainData<DT> plaindata_t;
	typedef typename DataGroup<data_t,ODIMPathElem::DATA>::datagroup_t datagroup_t; //  ODIMPathElem::DATA>
	typedef typename datagroup_t::map_t  map_t;


	inline
	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	//DataSet(typename data_t::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
	DataSet(typename data_t::tree_t & tree, const QuantitySelector & slct =QuantitySelector()) :
		//datagroup_t(tree, quantityRegExp), QualityDataSupport<DT>(tree)
		datagroup_t(tree, slct), QualityDataSupport<DT>(tree)
		{
	}

	DataSet(const DataSet<DT> & ds) : datagroup_t(ds), QualityDataSupport<DT>(ds) {
	}

	~DataSet(){

		drain::Logger mout(__FILE__, __FUNCTION__);

		switch (this->size()) {
		case 0:
			mout.debug3("no data<n> groups" );
			break;
		default:
			mout.info("several Data groups, using: " , this->begin()->first );
			// no break;
		case 1:
			mout.debug("updating from 1st data: " , this->begin()->first );


			const typename DT::odim_t & odim = this->getFirstData().odim;

			/*
			 // DEBUGGING 2024
			for (const auto & entry: this->baseODIM){
				if (odim[entry.first] == entry.second){
					mout.reject("BaseODIM differs: ", entry.first, ": ", entry.second, " vs ", odim[entry.first]);
				}
			}
			*/

			// updateTree3(this->getFirstData().odim);
			// mout.attention("start updateTree3");
			updateTree3(odim);
			// mout.attention("end updateTree3");
		}

	}


	plaindata_t & getQualityData2(const std::string & quantity = "QIND", const std::string & dataQuantity = ""){ // , bool tmp = false
		if (dataQuantity.empty())
			return this->getQualityData(quantity);
		else {
			return this->getData(dataQuantity).getQualityData(quantity);
		}
	}


	const plaindata_t & getQualityData2(const std::string & quantity = "QIND", const std::string & dataQuantity = "") const { // , bool tmp = false
		if (dataQuantity.empty())
			return this->getQualityData(quantity);
		else {
			return this->getData(dataQuantity).getQualityData(quantity);
		}
	}


	inline
	void updateTree3(const typename DT::odim_t & odim){  //
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATASET>(odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // TEST2019/09 // images, including DataSet.data, TODO: skip children
		//DataTools::updateInternalAttributes(this->tree, drain::FlexVariableMap()); // TEST2019/09 // images, including DataSet.data, TODO: skip children
	}

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename DT::odim_t & odim) const {  //
		std::cout << "updateTree3 const \n";
		//ODIM::updateH5AttributeGroups<ODIMPathElem::DATASET>(odim, tree);
	}



protected:



	/// Adds Data<DT>
	/**
	 *   \param dataPath - "data1", "data2", etc
	 *   \param quantityKey - "DBZH", "VRAD", etc.
	//Data<DT> & add(const std::string & dataPath, const std::string & quantityKey){
	Data<DT> & add(const ODIMPathElem & dataPath, const std::string & quantityKey){
		//typename DT::tree_t & t = this->tree(dataPath);
		typename DT::tree_t & t = this->tree[dataPath];
		typename DataSet<DT>::iterator it;
		it = this->insert(this->begin(), std::pair<std::string, Data<DT> >(quantityKey, Data<DT>(t, quantityKey)));
		return it->second;
	}
	 */




};

/// Dumps quantities and geometry
/**
 *
 */
template <typename DT>
std::ostream & operator<<(std::ostream & ostr, const DataSet<DT> & d){
	typedef DataSet<DT> dataset_t;
	ostr << "dataSet ";
	char separator = 0;
	drain::image::Geometry g;
	for (typename dataset_t::const_iterator it = d.begin(); it != d.end(); ++it){
		if (separator)
			ostr << separator;
		else {
			separator = ',';
			g.setGeometry(it->second.data.getGeometry());
		}
		ostr << it->first << '[' << drain::Type::getTypeChar(it->second.data.getType()) << ']';
		if (it->second.hasQuality())
			ostr << "+Q(" << it->second.getQuality() << ')';
	}
	ostr << " ("<< g << ")";
	if (d.hasQuality())
		ostr << " +quality(" << d.getQuality() << ')';
	/*
	ostr << d.data << ", ODIM:\t ";
	ostr << d.odim << '\n';
	ostr << "props: " << d.data.properties << '\n';
	*/
	return ostr;
}


/// Structure for storing sweeps by their elevation angle.
///  Note: this approach fails if a volume contains several azimuthal sweeps with a same elevation angle.
// Becoming DEPRECATED
//template <typename DT>
//class DataSetMap : public std::map<double, DataSet<DT> > {
//};

// TIMESTAMPED (what::date + what::time data) or path?
template <typename DT>
class DataSetMap : public std::map<std::string, DataSet<DT> > {
};



template <typename DT>
class DataSetList : public std::list<DataSet<DT> > {
};




} // rack::

namespace drain {

template <class T, typename TI, typename D>
struct TypeName<rack::DataType<T,TI,D> > {

	static const std::string & str(){
		static std::string name = drain::StringBuilder<>("DataType<", drain::TypeName<T>::str(), ',', drain::TypeName<TI>::str(), ',', drain::TypeName<D>::str(), ">");
		return name;
	}

};

}


#endif /* DATA_H_ */

