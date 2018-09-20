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

#include <set>
#include <list>
#include <map>

#include <drain/util/ReferenceMap.h>
#include <drain/util/RegExp.h>
#include <drain/util/Variable.h>
// #include <drain/util/Path.h>

//#include "ODIM.h"
#include "PolarODIM.h"
#include "CartesianODIM.h"
#include "VerticalODIM.h"
//#include "product/DataConversionOp.h"

#include "DataSelector.h"
#include "DataTools.h"

using namespace drain::image;

namespace rack {

/**

 Consider that const HI5TREE src or HI5TREE dst contain following structure:

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

  The classes in this section provide solutions for handling dataset-level and data-level information.

 \endcode


 */


/// Container that couples together trees (drain::Tree) and images (drain::image::Image). Source or destinations types.
/**
 *  The template parameters are expected to be exactly the following, all of them const or non-const:
 *  \tparam T - HDF5 tree, as HI5TREE
 *  \tparam TI - two-dimensional data, as drain::image::Image
 *  \tparam D - metadata, as rack::PolarODIM
 */
template <typename T, typename TI, typename D>
struct DataType {
	typedef T  tree_t;
	typedef TI tree_iter_t;
	typedef D  image_t;
};


/// Read-only data type with metadata
/**
 *  \tparam M - metadata using OPERA data information model
 */
template <typename M = PolarODIM const>
struct SrcType : public DataType<HI5TREE const, HI5TREE::const_iterator, Image const> {
	typedef M odim_t;
};


/// Writable data type with metadata
/**
 *  \tparam M - metadata using OPERA data information model
 */
template <typename M = PolarODIM>
struct DstType : public DataType<HI5TREE, HI5TREE::iterator, Image> {
	typedef M odim_t;
};


/// Read-only data in polar coordinates
typedef SrcType<PolarODIM const> PolarSrc;

/// Read-only data in Cartesian coordinates
typedef SrcType<CartesianODIM const> CartesianSrc;

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
 *  \tparam DT - metadata using OPERA data information model
 */
template <typename DT>
class DataObject {
public:

	typedef typename DT::tree_t tree_t;
	// typedef typename DT::image_t image_t;
	// typedef typename DT::odim_t odim_t;


//protected:

	/// General HDF5 data structure
	tree_t & tree;

public:

	//image_t & data;
	// Metadata structure
	//odim_t odim;
	//typename DT::odim_t odim;

protected:


	DataObject(tree_t & tree) :
		tree(tree)
		//data(tree["data"].data.dataSet),
		//odim(data, data.properties.get("what:quantity",""))
	{};

	/*
	DataObject(tree_t & tree, const std::string & quantity) :
		tree(tree),
		data(tree["data"].data.dataSet),
		odim(data, quantity)
	{};
	*/

	//DataObject(tree_t & tree, const odim_t & odim) tree(tree), odim(odim) {};
	DataObject(const DataObject & d) : tree(d.tree){}; //, image(d.data), odim(d.odim) {};


};

template <typename DT>
class RootData : public DataObject<DT> {

public:

	RootData(typename DT::tree_t & tree) : DataObject<DT>(tree) {
		// todo: init odim
	};

	virtual inline
	~RootData(){
		ODIM::copyToH5<ODIM::ROOT>(this->odim, this->tree);
		DataTools::updateAttributes(this->tree);
	};

	typename DT::odim_t odim;

};



/**
 *  The template parameters are expected to be exactly the following, but all of them const or non-const:
 *  \tparam T - HDF5 tree, as HI5TREE
 *  \tparam D - two-dimensional data, as drain::image::Image
 *  \tparam M - metadata, as rack::PolarODIM
 */
template <typename DT>
class PlainData : public DataObject<DT> {
public:

	typedef typename DT::tree_t tree_t;
	typedef typename DT::image_t image_t;
	typedef typename DT::odim_t odim_t;

	// NEW
	inline
	PlainData(typename DT::tree_t & tree) : DataObject<DT>(tree),
			data(tree["data"].data.dataSet),
			odim(data, data.properties.get("what:quantity",""))
			{
	}

	/// Constructor referring to HDF5 structure
	PlainData(typename DT::tree_t & tree, const std::string & quantity) : DataObject<DT>(tree),
		//tree(tree),
		data(tree["data"].data.dataSet),
		odim(data, quantity) // reads data.properties?
	{
		//odim.quantity = quantity;
	}

	/// Copy constructor, also for referencing non-const as const.
	/**
	 *   Compiler returns error if odim types ar incompatible.
	 */
	template <typename DT2>
	PlainData(const PlainData<DT2> & d) : DataObject<DT>(d.tree),
		data(this->tree["data"].data.dataSet),
		odim(d.odim)
		//odim(data, data.properties.get("what:quantity",""))
	{
		//odim.updateFromMap(d.odim);
	}

	inline
	~PlainData(){
		//drain::Logger mout("PlainData", __FUNCTION__);
		//mout.debug(1) << "calling updateTree2, odim: " << odim << mout.endl;
		updateTree2();
	}

	/// Saves type and sets the type of the actual data array as well.
	// TODO: rename to: setEncoding
	template <class T>
	inline
	void setEncoding(const T & type, const std::string & values = ""){
		//odim.type = drain::Type::getTypeChar(type);
		odim.type = drain::Type::getTypeChar(drain::Type(type));
		odim.setTypeDefaults(type, values);
		data.setType(type);
		//data.setGeometry(data.);
	}

	template <class DT2>
	inline
	void copyEncoding(const PlainData<DT2> & srcData){
		odim.importMap(srcData.odim);
		data.setType(odim.type);
		data.setScaling(odim.gain, odim.offset);
		//data.setGeometry(data.);
	}

	inline
	void setPhysicalRange(double min, double max){
		data.setPhysicalScale(min, max);
		odim.gain   = data.getScaling().scale;
		odim.offset = data.getScaling().offset;
	}

	/// Sets dimensions of data array and metadata.
	inline
	void setGeometry(size_t cols, size_t rows){
		odim.setGeometry(cols, rows);
		data.setGeometry(cols, rows);
	}

	inline
	void setGeometry(const drain::image::AreaGeometry & geometry){
		odim.setGeometry(geometry.getWidth(), geometry.getHeight());
		data.setGeometry(geometry);
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

	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	void setNoSave(bool noSave = true){ this->tree.data.noSave = noSave;};



	image_t & data;

	// Metadata structure
	odim_t odim;

	/// TODO: consider this to destructor
	inline
	void updateTree2(){
		//if (! DataTools::removeIfNoSave(this->tree)){
		ODIM::copyToH5<ODIM::DATA>(odim, this->tree);
		DataTools::updateAttributes(this->tree);
		//}
	}



	// Possibly this should be somewhere else? (Too specific here?)
	void createSimpleQualityData(drain::image::Image & qualityImage, double dataQuality=1.0, double nodataQuality=0.0, double undetectQuality=0.5) const;

	// Possibly this should be somewhere else? (Too specific here?)
	inline
	void createSimpleQualityData(PlainData<DT> & qualityData, double dataQuality=1.0, double nodataQuality=0.0, double undetectQuality=0.5) const { //, double dataQuality=1.0, double nodataQuality=0.0) const {
		qualityData.setEncoding(typeid(unsigned char));
		// qualityData.setGeometry(data.getWidth(), data.getHeight());
		createSimpleQualityData(qualityData.data, dataQuality, nodataQuality, undetectQuality);
		qualityData.odim.gain   = qualityData.data.getScaling().scale;
		qualityData.odim.offset = qualityData.data.getScaling().offset;
	}


protected:


};

template <typename DT>  // PlainData<DT> & quality
void PlainData<DT>::createSimpleQualityData(drain::image::Image & quality, double dataQuality, double nodataQuality, double undetectQuality) const {

	quality.setPhysicalScale(0.0, 1.0);
	const drain::image::ImageScaling & scaling = quality.getScaling();
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

}




template <typename DT>
inline
std::ostream & operator<<(std::ostream & ostr, const PlainData<DT> & d){
	ostr << d.data << ", ODIM:\t ";
	ostr << d.odim << '\n';
	ostr << "props: " << d.data.properties << '\n';
	return ostr;
}



/// Base class providing quality support for Data<DT> and DataSet<DT>
template <typename DT>
class QualityDataSupport {

public:

	typedef PlainData<DT> plaindata_t;

	virtual inline
	~QualityDataSupport(){};

	/// Returns true if non-empty, associated QIND data exists.
	inline
	bool hasQuality(const std::string & qualityQuantity = "QIND") const {
		return hasQualityData(qualityDataMap, qualityQuantity, getTree());
		//return !getQualityDataSupport(quantity).data.isEmpty();
	};


	/// Finds local quality data; maybe empty.
	/*
	 *  Returns local, "own" quality. Ie. if data is under data[i], returns data[i]/quality[j] for which quantity=[quantity]
	 *  \param quantity - qua
	 *  \param tmp - create temporary data array that will not be stored by Hi5Writer().
	 */
	plaindata_t & getQualityData(const std::string & quantity = "QIND") {
		return retrieveQualityData(qualityDataMap, quantity, getTree());
	}

	/// Finds local quality data; maybe empty.
	/*
	 *  Returns local, "own" quality. Ie. if data is under data[i], returns data[i]/quality[j] for which quantity=[quantity]
	 *  \param quantity - qua
	 *  \param tmp - create temporary data array that will not be stored by Hi5Writer().
	 */
	const plaindata_t & getQualityData(const std::string & quantity = "QIND") const { // , bool tmp = false
		return retrieveQualityData(qualityDataMap, quantity, getTree());  // , tmp
	}

	/// Retrieve general or quantity-specific quality data.
	/**
	 *  \param quantityQuality - name of the quality quantity to be searched, eg. "QIND".
	 *  \param quantity - quantity (like "DBZH"), the quality of which is searched
	 */

protected:

	static
	void findExistingQualityPath(const std::string & quantity, const typename plaindata_t::tree_t & tree, std::string & path){
		// "/quality.." =>
		// "^/quality.." =>
		DataSelector selector("^/quality[0-9]+$", std::string("^")+quantity+std::string("$"));
		DataSelector::getPath(tree, selector, path);
	}

	static
	bool hasQualityData(std::map<std::string, plaindata_t > & cacheMap, const std::string & quantity, const typename plaindata_t::tree_t & tree){

		if (cacheMap.find(quantity) != cacheMap.end())
			return true;
		else {
			std::string path = "";
			findExistingQualityPath(quantity, tree, path);
			return !path.empty();
		}

	}


	static
	plaindata_t & retrieveQualityData(std::map<std::string, plaindata_t > & cacheMap, const std::string & quantity, typename plaindata_t::tree_t & tree){ //  , bool tmp = false

		typename std::map<std::string, plaindata_t >::iterator it = cacheMap.find(quantity);

		if (it != cacheMap.end()){
			// std::cerr << "static Data::getQualityData(): cacheMap had '"<< quantity << "' already cached, ok" << std::endl;
			return it->second;
		}
		else {
			// "/quality.." =>
			// "^/quality.." =>
			std::string path = "";
			findExistingQualityPath(quantity, tree, path);
			if (path.empty()){  // create it
				// "/quality.." =>
				// "^/quality.." =>
				path = "quality1";
				DataSelector::getNextOrdinalPath(tree, "^/?quality[0-9]+$", path);
				// std::cerr << "PlainData::getQualityData(): creating '"<< quantity << "' in '"<< path <<"', caching" << std::endl;
			}

			it = cacheMap.insert(cacheMap.begin(), typename std::map<std::string, plaindata_t >::value_type(quantity, plaindata_t(tree(path), quantity)));  // WAS [path]
			// it = cacheMap.insert(cacheMap.begin(), typename plaindata_t(quantity, plaindata_t(tree[path])));

			return it->second;
		}
	}





protected:

	virtual
	const typename DT::tree_t & getTree() const = 0;

	virtual
	typename DT::tree_t & getTree() = 0;

	mutable
	std::map<std::string, plaindata_t > qualityDataMap;




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
 *  The template parameters are expected to be exactly the following, but all of them const or non-const:
 *  \tparam DT - data type (Polar|Cartesian)(Src|Dst)
 */
//template <typename T, typename D, typename M = PolarODIM>
template <typename DT>
class Data : public PlainData<DT>, public QualityDataSupport<DT> {
public:

	typedef PlainData<DT> plaindata_t;

	Data(typename DT::tree_t & tree) : PlainData<DT>(tree){
	}

	Data(typename DT::tree_t & tree, const std::string & quantity) : PlainData<DT>(tree, quantity){
	}

	// Data(const HI5TREE & src, const std::string & quantity = "^DBZH$");
	virtual ~Data(){};

protected:

	virtual inline
	const typename PlainData<DT>::tree_t & getTree() const { return this->tree; };

	virtual inline
	typename PlainData<DT>::tree_t & getTree(){ return this->tree; } ;


};





template <typename DT>
inline
std::ostream & operator<<(std::ostream & ostr, const Data<DT> & d){
	ostr << d.data << ", ODIM:\t ";  // .getGeometry()
	ostr << d.odim; // << '\n';
	//ostr << d.data.properties << '\n';
	if (d.hasQuality()){
		ostr << " (has quality field)";
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
class DataSet : public std::map<std::string, Data<DT> >, public QualityDataSupport<DT> { // typename T::data_t
public:

	typedef PlainData<DT> plaindata_t;

	typedef std::map<std::string, Data<DT> > map_type;

	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataSet(typename DT::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		tree(init(tree, quantityRegExp, *this)) {
	}

	virtual
	~DataSet(){
		drain::Logger mout("DataSet", __FUNCTION__);
		switch (this->size()) {
			case 0:
				mout.warn() << "no data<n> groups" << mout.endl;
				break;
			default:
				mout.info() << "several Data groups, using: " << this->begin()->first << mout.endl;
				// no break;
			case 1:
				mout.debug() << "updating from 1st data: " << this->begin()->first << mout.endl;
				updateTree3(getFirstData().odim); // tree
		}
	};


	// Metadata structure (not needed, first data instead?)
	//typename DT::odim_t odim;

	typename DT::tree_t & tree;  // DON'T HACK. TO BE MOVED protected FOR SUBCLASSES?

	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	void setNoSave(bool noSave = true){ this->tree.node.noSave = noSave;};

	Data<DT> & getFirstData() {

		const typename DataSet<DT>::iterator it = this->begin();

		if (it == this->end()){
			drain::Logger mout("DataSetDst", __FUNCTION__);
			mout.error() << "no data" << mout.endl;
			return getData("");
		}
		else
			return it->second;

	}

	const Data<DT> & getFirstData() const {

		const typename DataSet<DT>::const_iterator it = this->begin();

		if (it == this->end()){
			drain::Logger mout("DataSetSrc", __FUNCTION__);
			mout.error() << "no data" << mout.endl;
			return getEmpty();
		}
		else
			return it->second;

	}

	/// Retrieves data containing the given quantity. If not found, returns an empty array.
	const Data<DT> & getData(const std::string & quantityKey) const {

		const DataSet<DT> & src = *this;

		const typename DataSet<DT>::const_iterator it = src.find(quantityKey);

		if (it != src.end()){
			return it->second;
		}
		else {
			return getEmpty();
		}
	}

	const Data<DT> & getData(const drain::RegExp & quantity) const {

		const DataSet<DT> & src = *this;
		for (typename DataSet<DT>::const_iterator it = src.begin(); it != src.end(); ++it){
			if (quantity.test(it->first)){
				return it->second;
			}
		}

		return getEmpty();

	}

	/// Retrieves data containing the given quantity. If not found, creates an array.
	Data<DT> & getData(const std::string & quantityKey) { //

		drain::Logger mout("DataSetDst", __FUNCTION__);

		//DataSetDst<D> & dst = *this;
		//typename DataSetDst<DT>::iterator it = this->find(quantityKey);
		typename DataSet<DT>::iterator it = this->find(quantityKey);

		if (it != this->end()){
			return it->second;
		}
		else {
			if (quantityKey.empty()){
				mout.warn() << " quantityKey empty" << mout.endl;
			}

			std::string dataPath = "data1";  // hence finally "data<n>" //// or "quality<1>"
			DataSelector::getNextOrdinalPath(this->tree, "data[0-9]+/?$", dataPath);

			return add(dataPath, quantityKey);
			/*
			typename DT::tree_t & t = this->tree(dataPath);
			it = this->insert(this->begin(), std::pair<std::string, Data<DT> >(quantityKey, Data<DT>(t, quantityKey)));
			// TODO: it->second.odim.quantity = quantity ?
			return it->second;
			*/
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


	/*
	Data<DT> & clone(const std::string & quantity) {
		drain::Logger mout("DataSetDst", __FUNCTION__);
		std::stringstream sstr;
		sstr << '^' << quantity << '$';
		const DataSelector selector("data[0-9]$", sstr.str());
		std::string path;
		if (!DataSelector::getPath(tree, selector, path)){
			mout.error() << "failed, quantity " << quantity << " not found " << mout.endl;
		}
		return add(path, quantity);
	}
	*/

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename DT::odim_t & odim){  //
		//odim.copyToDataSet(this->tree);
		//if (!DataTools::removeIfNoSave(this->tree))
		ODIM::copyToH5<ODIM::DATASET>(odim, this->tree);
		DataTools::updateAttributes(this->tree); // images, including DataSet.data, TODO: skip children
	}

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename DT::odim_t & odim) const {  //
		std::cout << "updateTree3 const \n";
		//ODIM::copyToH5<ODIM::DATASET>(odim, tree);
	}




protected:

	static
	const Data<DT> & getEmpty() {
		static typename Data<DT>::tree_t t;
		static Data<DT> empty(t);
		return empty;
	}

	/// Adds Data<DT>
	/**
	 *   \param dataPath - "data1", "data2", etc
	 *   \param quantityKey - "DBZH", "VRAD", etc.
	 */
	Data<DT> & add(const std::string & dataPath, const std::string & quantityKey){
		typename DT::tree_t & t = this->tree(dataPath);
		typename DataSet<DT>::iterator it;
		it = this->insert(this->begin(), std::pair<std::string, Data<DT> >(quantityKey, Data<DT>(t, quantityKey)));
		return it->second;
	}


	// For QualityData
	virtual inline
	const typename PlainData<DT>::tree_t & getTree() const { return this->tree; };

	// For QualityData
	virtual inline
	typename PlainData<DT>::tree_t & getTree(){ return this->tree; } ;


	/// Given a root, constructs a map of data sets of desired quantities.
	static
	typename DT::tree_t & init(typename DT::tree_t & root, const DataSelector & selector, DataSet<DT> & dst){
		DataSelector s(selector);
		s.quantity.clear();
		std::string path = "dataset1";
		DataSelector::getPath(root, s, path);
		return init(root(path), drain::RegExp(selector.quantity), dst);
	}

	/// Given DataSet subtree, like tree["dataset3"], constructs a data map of desired quantities.
	static
	typename DT::tree_t & init(typename DT::tree_t & datasetTree, const drain::RegExp & quantityRegExp, DataSet<DT> & dst){

		/// ODC non-standard quantity location
		//const std::string datasetQuantity = datasetTree["what"].data.attributes["quantity"];
		//const std::string datasetQuantity = datasetTree["what"].data.attributes.get("quantity", std::string(""));

		const std::string datasetQuantity = datasetTree["what"].data.attributes.get("quantity", "");

		for (typename DT::tree_iter_t it=datasetTree.begin(); it!=datasetTree.end(); ++it){

			//if  ((it->first == "how") || (it->first == "what") || (it->first == "where"))
			if (EncodingODIM::attributeGroups.find(it->first) != EncodingODIM::attributeGroups.end())
				continue;

			if  (it->first.find("quality") == 0){  // starts with "quality"
				continue;
			}


			const std::string dataQuantity = it->second["what"].data.attributes["quantity"];  // Todo Variable cast const std::string &

			const std::string & quantity = !dataQuantity.empty() ? dataQuantity : datasetQuantity;

			if (quantity.empty() && quantityRegExp.toStr().empty()){
				drain::Logger mout("DataSet", __FUNCTION__);
				mout.warn() << "quantities dataset:'" << datasetQuantity << "', data:'" << dataQuantity << "'"<< mout.endl;
				mout.warn() << "undefined quantity in " << it->first << ", using key=" << it->first << mout.endl;
				// Assign by path component "data3"
				dst.insert(typename map_type::value_type(it->first, Data<DT>(it->second, it->first)));
				//associate(dst, it->first, it->second);
			}
			else if (quantityRegExp.test(quantity) ){
				if (dst.find(quantity) != dst.end()){ // already created
					drain::Logger mout("DataSet", __FUNCTION__);
					mout.warn() << "quantity '" << quantity << "' replaced same quantity at " << it->first << mout.endl;
				}
				dst.insert(typename map_type::value_type(quantity, Data<DT>(it->second, quantity)));
				//dst[quantity] = T(it->second);
				//associate(dst, quantity, it->second);
			}
			else if ((!quantity.empty() && (quantity.at(0) == '~') && quantityRegExp.test(quantity.substr(1)))){ // normalized  "~QUANTITY" experimental (scaled data standard)
				dst.insert(typename map_type::value_type(quantity, Data<DT>(it->second, quantity)));
				//associate(dst, quantity, it->second);
			}
		}

		return datasetTree;
	};


	// Risen to QualityData
	// mutable
	// std::map<std::string, typename Data<DT>::plaindata_t > qualityDataMap;

};


/// Structure for storing sweeps by their elevation angle.
///  Note: this approach fails if a volume contains several azimuthal sweeps with a same elevation angle.
// Becoming DEPRECATED
template <typename DT>
class DataSetMap : public std::map<double, DataSet<DT> > {
};


template <typename DT>
class DataSetList : public std::list<DataSet<DT> > {
};




} // rack::

#endif /* DATA_H_ */

// Rack
