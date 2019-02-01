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


/// Container that couples together a tree structure and a data array (drain::image::Image), according to source type (const or non-const) .
/**
 *  The template parameters are expected to be exactly the following, all of them const or non-const:
 *  \tparam T  - HDF5 tree, as HI5TREE
 *  \tparam TI - tree iterator type
 *  \tparam D  - data array (drain::image::Image)
 */
template <typename T, typename TI, typename D>
struct DataType {
	typedef T  tree_t;
	typedef TI tree_iter_t;
	typedef D  image_t;
};


/// Read-only data type
/**
 *  \tparam M - meta data using OPERA data information model (ODIM)
 */
template <typename M = PolarODIM const>
struct SrcType : public DataType<HI5TREE const, HI5TREE::const_iterator, Image const> {
	typedef M odim_t;
};


/// Writable data type
/**
 *  \tparam M - meta data using OPERA data information model (ODIM)
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
 *  \tparam DT - data type: SrcType<> or DstType<>
 */
template <typename DT>
class TreeWrapper {
public:

	typedef DT datatype_t;
	typedef typename DT::tree_t tree_t;
	typedef typename DT::tree_iter_t tree_iter_t;
	// typedef typename DT::image_t image_t;
	// typedef typename DT::odim_t odim_t;

	inline
	const drain::VariableMap & getWhat() const {
		//return getTree()[ODIMPathElem(ODIMPathElem::WHAT)].data.attributes;
		return getAttr<ODIMPathElem::WHAT>();
	}

	inline
	drain::VariableMap & getWhat() {
		return getAttr<ODIMPathElem::WHAT>();
		//return getTree()[ODIMPathElem(ODIMPathElem::WHAT)].data.attributes;
	}

	inline
	const drain::VariableMap & getWhere() const {
		return getAttr<ODIMPathElem::WHERE>();
		//return getTree()[ODIMPathElem(ODIMPathElem::WHERE)].data.attributes;
	}

	inline
	drain::VariableMap & getWhere() {
		return getAttr<ODIMPathElem::WHERE>();
		//return getTree()[ODIMPathElem(ODIMPathElem::WHERE)].data.attributes;
	}

	/*
	inline
	int & test(){
		static int dummy = 1;
		return dummy;
	}

	inline
	const int & test() const {
		static int dummy = 1;
		return dummy;
	}
	*/

	inline
	const drain::VariableMap & getHow() const {
		return getAttr<ODIMPathElem::HOW>();
		//return getTree()["how"].data.attributes;
	}

	inline
	drain::VariableMap & getHow() { // ODIMPathElem(ODIMPathElem::HOW)
		return getAttr<ODIMPathElem::HOW>();
		// return getTree()["how"].data.attributes;
	}



//protected:

	virtual inline
	const tree_t & getTree() const { return this->tree; };

	virtual inline
	tree_t & getTree(){ return this->tree; } ;


protected:

	/// General HDF5 data structure
	tree_t & tree;

	TreeWrapper(tree_t & tree) : tree(tree){
	};

	//TreeWrapper(tree_t & tree, const odim_t & odim) tree(tree), odim(odim) {};
	TreeWrapper(const TreeWrapper & d) : tree(d.tree){}; //, image(d.data), odim(d.odim) {};

	template <ODIMPathElem::group_t G>
	const drain::VariableMap & getAttr() const {
		return this->tree[ODIMPathElem(G)].data.attributes;
	}

	template <ODIMPathElem::group_t G>
	drain::VariableMap & getAttr(){
		return this->tree[ODIMPathElem(G)].data.attributes;
	}

};

template <typename DT>
class RootData : public TreeWrapper<DT> {

public:

	RootData(typename DT::tree_t & tree) : TreeWrapper<DT>(tree) {
		// todo: init odim
	};

	virtual inline
	~RootData(){
		ODIM::copyToH5<ODIMPathElem::ROOT>(this->odim, this->tree);
		DataTools::updateAttributes(this->tree); // overrides anything?
	};

	typename DT::odim_t odim;

};


/// Essential class for storing radar data.
/**
 *  \tparam DT - data type (PolarSrc, PolarDst, CartesianSrc, CartesianDst, ...)
 */
template <typename DT>
class PlainData : public TreeWrapper<DT> {
public:

	typedef typename DT::tree_t tree_t;
	typedef typename DT::image_t image_t;
	typedef typename DT::odim_t odim_t;

	// NEW
	inline
	PlainData(typename DT::tree_t & tree) : TreeWrapper<DT>(tree),
	data(tree["data"].data.dataSet),
	odim(data, data.properties.get("what:quantity",""))
	{
		//data.setScaling(odim.gain, odim.offset);
	}

	/// Constructor referring to HDF5 structure
	PlainData(typename DT::tree_t & tree, const std::string & quantity) : TreeWrapper<DT>(tree),
			data(tree["data"].data.dataSet),
			odim(data, quantity) // reads data.properties?
			{

		//data.setScaling(odim.gain, odim.offset);
			}

	/// Copy constructor, also for referencing non-const as const.
	/**
	 *   Compiler returns error if odim types ar incompatible.
	 */
	template <typename DT2>
	PlainData(const PlainData<DT2> & d) : TreeWrapper<DT>(d.getTree()),
		data(this->tree["data"].data.dataSet),
		odim(d.odim)
	{
		//data.setScaling(odim.gain, odim.offset);
	}

	inline
	~PlainData(){
		//drain::Logger mout("PlainData", __FUNCTION__);
		//mout.debug(1) << "calling updateTree2, odim: " << odim << mout.endl;
		updateTree2();
	}

	/// Saves type and sets the type of the actual data array as well.
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



	// Possibly this should be somewhere else? (Too specific here?)
	/// For this data, creates an on-off quality data.
	void createSimpleQualityData(drain::image::Image & qualityImage, double dataQuality=1.0, double nodataQuality=0.0, double undetectQuality=0.5) const;

	// Possibly this should be somewhere else? (Too specific here?)
	/// For this data, creates an on-off quality data.
	inline
	void createSimpleQualityData(PlainData<DT> & qualityData, double dataQuality=1.0, double nodataQuality=0.0, double undetectQuality=0.5) const { //, double dataQuality=1.0, double nodataQuality=0.0) const {
		qualityData.setEncoding(typeid(unsigned char));
		createSimpleQualityData(qualityData.data, dataQuality, nodataQuality, undetectQuality);
		qualityData.odim.gain   = qualityData.data.getScaling().scale;
		qualityData.odim.offset = qualityData.data.getScaling().offset;
	}

	/// TODO: consider this to destructor
	inline
	void updateTree2(){
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, this->tree);
		DataTools::updateAttributes(this->tree);
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


/// Something, that contains TreeWrapper that can be retrieved by quantity keys.
/**
    \tparam DDT  - datatype: PlainData<> or Data<>

   See SweepSrc and ProductDst below.
 */
template <class D, ODIMPathElem::group_t G>
class DataGroup : public TreeWrapper<typename D::datatype_t>, public std::map<std::string, D > { // typename T::datatype_t
public:

	typedef D data_t;
	typedef typename D::datatype_t datatype_t;
	typedef std::map<std::string, D > map_t;
	typedef DataGroup<data_t,G> datagroup_t;

	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataGroup(typename D::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		TreeWrapper<typename D::datatype_t>(tree) {
		init(tree, *this, quantityRegExp);
	}

	DataGroup(const datagroup_t & src) : TreeWrapper<typename D::datatype_t>(src.tree) {
		//adapt(src.tree, *this, src);  // ALERT: includes all the quantities, even thoug src contained only some of them
		adapt(src, *this);  // ALERT: includes all the quantities, even thoug src contained only some of them
		//init(src.tree, *this);  // ALERT: includes all the quantities, even thoug src contained only some of them
	}


	virtual
	~DataGroup(){

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		switch (this->size()) {
		case 0:
			mout.debug(4) << "no data<n> groups" << mout.endl;
			break;
		default:
			mout.info() << "several Data groups, using: " << this->begin()->first << mout.endl;
			// no break;
		case 1:
			mout.debug() << "updating from 1st data: " << this->begin()->first << mout.endl;
			updateTree3(this->getFirstData().odim); // tree
		}

	};


	const data_t & get(const std::string & quantity) const {

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G) + " {const}", __FUNCTION__);

		//mout.warn() << "const " << mout.endl;

		typename datagroup_t::const_iterator it = this->find(quantity);

		if (it != this->end()){
			mout.debug(2) << '[' << quantity << "]\t = " << it->first << mout.endl;
			return it->second;
		}
		else {
			mout.debug() << '[' << quantity << "] not found, returning empty"  << mout.endl;
			return getEmpty();
		}
	}


	data_t & get(const std::string & quantity) {

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);

		typename datagroup_t::iterator it = this->find(quantity);

		//mout.warn() << "non-const " << mout.endl;

		if (it != this->end()){
			mout.debug(4) << "found " << it->first << mout.endl;
			return it->second;
		}
		else {
			//mout.note() << "not found..." << mout.endl;
			ODIMPathElem child(G);
			DataSelector::getNextChild(this->tree, child);
			mout.debug(2) << "add: " << child << " [" << quantity << ']' << mout.endl;
			it = this->insert(this->begin(), typename map_t::value_type(quantity, D(this->getTree()[child], quantity)));  // WAS [path]
			//it->second.
			//return add(child, quantityKey);
			return it->second;
		}

	}

	const data_t& get(const drain::RegExp & regExp) const {

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G)+"(RegExp) {const}", __FUNCTION__);

		//mout.warn() << "const " << mout.endl;

		for (typename datagroup_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (regExp.test(it->first)){
				mout.debug() << "quantity " << it->first << " matches " << regExp << mout.endl;
				return it->second;
			}
		}

		mout.note() << "no quantity match for " << regExp << mout.endl;

		return getEmpty();

	}

	data_t & getFirstData() {

		const typename datagroup_t::iterator it = this->begin();

		if (it == this->end()){
			drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
			mout.error() << "no data" << mout.endl;
			return this->get("");
		}
		else
			return it->second;

	}

	const data_t & getFirstData() const {

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G) + " {const}", __FUNCTION__);

		//mout.warn() << "const" << mout.endl;

		typename datagroup_t::const_iterator it = this->begin();

		if (it != this->end()){
			mout.debug(1) << "found: " << it->first << mout.endl;
			return it->second;
		}
		else {
			mout.note() << "not found, returning empty"  << mout.endl;
			return getEmpty();
		}


	}


	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename datatype_t::odim_t & odim){  //
		//odim.copyToDataSet(this->tree);
		//if (!DataTools::removeIfNoSave(this->tree))
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, this->tree);
		DataTools::updateAttributes(this->tree); // images, including DataSet.data, TODO: skip children
	}

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename datatype_t::odim_t & odim) const {  //
		std::cout << "updateTree3 const \n";
		//ODIM::copyToH5<ODIMPathElem::DATASET>(odim, tree);
	}


protected:

	static
	const data_t & getEmpty() {
		static typename D::tree_t t;
		static data_t empty(t);
		return empty;
	}

	/// Given DataSet subtree, like tree["dataset3"], constructs a data map of desired quantities.
	/**
	 *   \param t   - target data tree
	 *   \param dst - odim wrapper for the data tree
	 */
	static
	typename D::tree_t & init(typename D::tree_t & t, datagroup_t & dst, const drain::RegExp & quantityRegExp = drain::RegExp()){

		// if (t.empty()) return; // no use, /data and /what groups still there, typically.

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);

		const bool USE_REGEXP = !quantityRegExp.toStr().empty();

		// Number of potential groups for debug note at end
		unsigned short counter = 0;

		mout.debug(3) << "collecting data items";
		if (USE_REGEXP)
			mout << ", RegExp=" << quantityRegExp.toStr();
		mout << mout.endl;

		// add UKMO
		const std::string datasetQuantity = t["what"].data.attributes.get("quantity", "");

		for (typename D::tree_iter_t it=t.begin(); it!=t.end(); ++it){

			if (! (it->first.is(G))){
				//mout.warn() << "skip '" << it->first << "' \t group != " << G << mout.endl;
				continue;
			}

			const std::string dataQuantity = it->second["what"].data.attributes["quantity"];

			const std::string & quantity = !dataQuantity.empty() ? dataQuantity : datasetQuantity;

			if (USE_REGEXP){
				++counter; // candidate count
				if (!quantityRegExp.test(quantity)){
					mout.debug(3) << "rejected '" << it->first << "' [" << quantity << "] !~" << quantityRegExp.toStr() << mout.endl;
					continue;
				}
			}

			mout.debug(3) << "accept '" << it->first << "' [" << quantity << ']' << mout.endl;


			if (quantity.empty()){
				//drain::Logger mout("DataSet", __FUNCTION__);
				mout.warn() << "quantities dataset:'" << datasetQuantity << "', data:'" << dataQuantity << "'"<< mout.endl;
				mout.warn() << "undefined quantity in " << it->first << ", using key=" << it->first << mout.endl;
				// Assign by path component "data3"
				dst.insert(typename map_t::value_type(it->first, D(it->second, it->first)));
				//associate(dst, it->first, it->second);
			}
			else {
				if (dst.find(quantity) != dst.end()){ // already created
					drain::Logger mout("DataSet", __FUNCTION__);
					mout.warn() << "quantity '" << quantity << "' replaced same quantity at " << it->first << mout.endl;
				}
				dst.insert(typename map_t::value_type(quantity, D(it->second, quantity)));
				//typename datagroup_t::reverse_iterator rit = dst.rend();
				//mout.warn() << "last '" << "' [" << quantity << '=' << rit->first << ']' << rit->second << mout.endl;
				//dst[quantity] = T(it->second);
				//associate(dst, quantity, it->second);t.
			}
		}

		if (USE_REGEXP)
			mout.debug(1) << "collected " << dst.size() << '/' << counter << " data items with RegExp=/" << quantityRegExp.toStr() << '/' << mout.endl;
		else
			mout.debug(1) << "collected " << dst.size() << " data items" << mout.endl;

		return t;
	};


	static
	//typename D::tree_t & adapt(typename D::tree_t & t, datagroup_t & dst, const datagroup_t & src){
	typename D::tree_t & adapt(const datagroup_t & src, datagroup_t & dst){

		drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);

		if (src.empty()){
			mout.debug(3) << "src empty" << mout.endl;
			return src.tree;
		}


		for (typename datagroup_t::const_iterator it=src.begin(); it!=src.end(); ++it){
			//dst.insert(typename map_t::value_type(it->first, D(it->second, it->first)));
			//dst.insert(typename map_t::value_type(it->first, D(it->second)));
			dst.insert(typename map_t::value_type(it->first, it->second));
		}
		mout.debug(2) << "adapted " << dst.size() << " data items; " << src.begin()->first << "..." << mout.endl;

		//return t
		return src.tree;
	};


};


/// Base class providing quality support for Data<DT> and DataSet<DT>
template <typename DT>
class QualityDataSupport {

public:

	typedef PlainData<DT> plaindata_t;

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
		return this->quality.get(quantity);
	}

	/// Finds associated quality data - maybe empty and unscaled.
	/*
	 *  \param quantity - quality quantity, "QIND" by default.
	 *  \return - \c data[i]/quality[j] for which \c quantity=[quantity]
	 *
	 */
	inline
	plaindata_t & getQualityData(const std::string & quantity = "QIND") {
		return this->quality.get(quantity);
	}

	inline
	bool hasQuality(const std::string & quantity = "QIND") const {
		return this->quality.find(quantity) != this->quality.end();
	}


protected:

	DataGroup<plaindata_t,ODIMPathElem::QUALITY> quality;

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

	// Data(const HI5TREE & src, const std::string & quantity = "^DBZH$");
	virtual ~Data(){};

protected:

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
class DataSet : public DataGroup<Data<DT>,ODIMPathElem::DATA>, public QualityDataSupport<DT> { // typename T::data_t
public:

	typedef Data<DT> data_t;
	typedef PlainData<DT> plaindata_t;
	typedef typename DataGroup<data_t,ODIMPathElem::DATA>::datagroup_t datagroup_t;
	typedef typename datagroup_t::map_t  map_t;


	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataSet(typename data_t::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		datagroup_t(tree, quantityRegExp), QualityDataSupport<DT>(tree)
		{
	}

	DataSet(const DataSet<DT> & ds) : datagroup_t(ds), QualityDataSupport<DT>(ds) {
	}



	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	void setNoSave(bool noSave = true){ this->tree.node.noSave = noSave;};



	/// Retrieves data containing the given quantity. If not found, returns an empty array.
	inline
	const data_t & getData(const std::string & quantity) const {
		return this->get(quantity);
	}

	/// Retrieves data containing the given quantity. If not found, creates an array.
	inline
	data_t & getData(const std::string & quantity) { //
		//drain::Logger mout("DataSetDst", __FUNCTION__);
		return this->get(quantity);
	}

	/// Retrieves data matching the given quantity. If not found, returns an empty array.
	inline
	const data_t & getData(const drain::RegExp & regExp) const {
		return this->get(regExp);
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


template <typename DT>
std::ostream & operator<<(std::ostream & ostr, const DataSet<DT> & d){
	ostr << "dataSet ";
	char separator = 0;
	drain::image::Geometry g;
	for (typename DataSet<DT>::const_iterator it = d.begin(); it != d.end(); ++it){
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

