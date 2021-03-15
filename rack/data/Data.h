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

#include "drain/util/ReferenceMap.h"
#include "drain/util/RegExp.h"
#include "drain/util/Variable.h"

//#include "drain/image/Legend.h"

#include "PolarODIM.h"
#include "CartesianODIM.h"
#include "VerticalODIM.h"

#include "DataSelector.h"
#include "DataTools.h"


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
 *  \tparam DT - data type: SrcType<> or DstType<> with ODIM template (PolarODIM, CartesianODIM)
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

	// Mark this data temporary so that it will not be save by Hi5::write().
	inline
	void setNoSave(bool noSave = true){ this->tree.data.noSave = noSave;};



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


	~TreeWrapper(){
		/*
		drain::Logger mout("TreeWrapper", __FUNCTION__);
		if (this->tree.data.noSave){
			mout.note() << "deleting (children only?)" << mout.endl;
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

template <typename DT>
class RootData : public TreeWrapper<DT> {

public:

	RootData(typename DT::tree_t & tree) : TreeWrapper<DT>(tree) {
		// todo: init odim
	};

	virtual inline
	~RootData(){
		ODIM::copyToH5<ODIMPathElem::ROOT>(this->odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // overrides anything?
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
		//data.setScaling(odim.scaling.scale, odim.scaling.offset);
	}

	/// Constructor referring to HDF5 structure
	PlainData(typename DT::tree_t & tree, const std::string & quantity) : TreeWrapper<DT>(tree),
			data(tree["data"].data.dataSet),
			odim(data, quantity) // reads data.properties?
			{
				//data.setScaling(odim.scaling.scale, odim.scaling.offset);
			}

	/// Copy constructor, also for referencing non-const as const.
	/**
	 *   Compiler returns error if odim types ar incompatible.
	PlainData(const PlainData<DT> & d) : TreeWrapper<DT>(d.getTree()),
		data(this->tree["data"].data.dataSet),
		odim(data)  // NEW
	{
	}
	 */

	template <typename DT2>
	PlainData(const PlainData<DT2> & d) : TreeWrapper<DT>(d.getTree()),
		data(this->tree["data"].data.dataSet),
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
		//mout.debug2() << "calling updateTree2, odim: " << odim << mout.endl;
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
		//data.setScaling(odim.scaling.scale, odim.scaling.offset); // needed?
		data.setScaling(odim.scaling); // needed?
		//data.setGeometry(data.);
	}

	inline
	void setPhysicalRange(double min, double max){
		//data.setPhysicalScale(min, max);
		data.setPhysicalRange(min, max, true);
		// data.setOptimalScale();
		odim.scaling.assign(data.getScaling());
		// odim.scaling.scale   = data.getScaling().scale; // needed?
		// odim.scaling.offset = data.getScaling().offset; // needed?
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


	// Data array
	image_t & data;

	// Metadata structure
	odim_t odim;

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
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, this->tree);
		DataTools::updateInternalAttributes(this->tree);
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
		if (UNDETECT && (*it == odim.undetect))
			*wit = undetectCode;
		else if (NODATA && (*it == odim.nodata))
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


/// Something, that contains TreeWrapper and data that can be retrieved by quantity keys.
/**
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

	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataGroup(typename DT::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		TreeWrapper<typename DT::datatype_t>(tree) {
		init(tree, *this, quantityRegExp);
	}

	DataGroup(const datagroup_t & src) : TreeWrapper<typename DT::datatype_t>(src.tree) {
		//adapt(src.tree, *this, src);  // ALERT: includes all the quantities, even thoug src contained only some of them
		adapt(src, *this);  // ALERT: includes all the quantities, even thoug src contained only some of them
		//init(src.tree, *this);  // ALERT: includes all the quantities, even thoug src contained only some of them
	}


	virtual
	~DataGroup(){
		/*
		drain::Logger mout("DataGroup<" + ODIMPathElem::getKey(G)+">", __FUNCTION__);
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
		*/
	};


	bool has(const std::string & quantity) const {
		return (this->find(quantity) != this->end());
	}


	const data_t & getData(const std::string & quantity) const {

		//drain::Logger mout(__FUNCTION__, __FILE__); //
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}");

		typename datagroup_t::const_iterator it = this->find(quantity);

		if (it != this->end()){
			mout.debug3() << '[' << quantity << "]\t = " << it->first << mout.endl;
			return it->second;
		}
		else {
			mout.debug() << '[' << quantity << "] not found, returning empty"  << mout.endl;
			return getEmpty();
		}
	}


	data_t & getData(const std::string & quantity) {

		//drain::Logger mout(__FUNCTION__, __FILE__); //REPL "DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}");

		//mout.warn() << "non-const " << mout.endl;
		typename datagroup_t::iterator it;
		#pragma omp critical  //(h5insert)
		{
			it = this->find(quantity);
			if (it != this->end()){
				mout.debug(4) << "found " << it->first << mout.endl;
			}
			else {
				//mout.note() << "not found..." << mout.endl;
				ODIMPathElem child(G);
				DataSelector::getNextChild(this->tree, child);
				mout.debug3() << "add: " << child << " [" << quantity << ']' << mout.endl;
				it = this->insert(this->begin(), typename map_t::value_type(quantity, DT(this->getTree()[child], quantity)));  // WAS [path]
			}
		}
		return it->second;

	}

	const data_t& getData(const drain::RegExp & regExp) const {

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G)+"(RegExp) {const}", __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

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

	/// Creates (or overrides) data array for \c quantity and scales it
	/**
	 *	\param quantity -
	 *	\param templateQuantity - predefined quantity the scaling of which is used in initialisation
	 *	\param encodingParams (optional) - parameters overriding those of template quantity
	 */
	data_t & create(const std::string & quantity, const std::string & templateQuantity, const std::string & encodingParams) {
		data_t & d = getData(quantity);
		d.setGeometry(0, 0); // in case existed already
		//getQuantityMap().setQuantityDefaults(d, templateQuantity, encodingParams);
		return d;
	}


	data_t & getFirstData() {

		const typename datagroup_t::iterator it = this->begin();

		if (it == this->end()){
			//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
			drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

			mout.error() << "no data" << mout.endl;
			return this->getData("");
		}
		else
			return it->second;

	}

	const data_t & getFirstData() const {

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G) + " {const}", __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		//mout.warn() << "const" << mout.endl;

		typename datagroup_t::const_iterator it = this->begin();

		if (it != this->end()){
			mout.debug2() << "found: " << it->first << mout.endl;
			return it->second;
		}
		else {
			mout.note() << "not found, returning empty"  << mout.endl;
			return getEmpty();
		}


	}


	// experimental
	data_t & getLastData() {

		const typename datagroup_t::reverse_iterator it = this->rend();

		if (it == this->rbegin()){
			drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");
			mout.error() << "no data" << mout.endl;
			return this->getData("");
		}
		else
			return it->second;

	}

	// experimental
	const data_t & getLastData() const {

		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G) + "}-const");

		//mout.warn() << "const" << mout.endl;

		typename datagroup_t::const_reverse_iterator it = this->rend();

		if (it != this->rbegin()){
			mout.debug2() << "found: " << it->first << mout.endl;
			return it->second;
		}
		else {
			mout.note() << "not found, returning empty"  << mout.endl;
			return getEmpty();
		}


	}


	// TODO: consider this to destructor!
	/*
	inline
	void updateTree3(const typename datatype_t::odim_t & odim){  //
		//odim.copyToDataSet(this->tree);
		//if (!DataTools::removeIfNoSave(this->tree))
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // images, including DataSet.data, TODO: skip children
	}

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename datatype_t::odim_t & odim) const {  //
		std::cout << "updateTree3 const \n";
		//ODIM::copyToH5<ODIMPathElem::DATASET>(odim, tree);
	}
	*/

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
	typename DT::tree_t & init(typename DT::tree_t & t, datagroup_t & dst, const drain::RegExp & quantityRegExp = drain::RegExp()){

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		const bool USE_REGEXP = quantityRegExp.isSet();

		// Number of potential groups for debug note at end
		unsigned short counter = 0;

		mout.debug(3) << "collecting data items";
		if (USE_REGEXP)
			mout << ", RegExp=" << quantityRegExp.toStr();
		mout << mout.endl;

		// #pragma omp critical //(h5insert2)
		{
			// add UKMO

			//const std::string datasetQuantity = t["what"].data.attributes.get("quantity", "");
			const std::string datasetQuantity = t[ODIMPathElem::WHAT].data.attributes.get("quantity", "");


			for (typename DT::tree_iter_t it=t.begin(); it!=t.end(); ++it){

				/// Accept groups of type G only
				if (! (it->first.is(G))){
					//mout.warn() << "skip '" << it->first << "' \t group != " << G << mout.endl;
					continue;
				}

				//const std::string dataQuantity = it->second["what"].data.attributes["quantity"];
				const std::string dataQuantity = it->second[ODIMPathElem::WHAT].data.attributes["quantity"];

				const std::string & quantity = !dataQuantity.empty() ? dataQuantity : datasetQuantity;

				if (USE_REGEXP){
					++counter; // candidate count
					if (!quantityRegExp.test(quantity)){
						//if (it->second.hasChild("quality1"))
						//	mout.warn() << it->first << "...rejecting, but has quality?" << mout.endl;
						mout.debug(3) << "rejected '" << it->first << "' [" << quantity << "] !~" << quantityRegExp.toStr() << mout.endl;
						continue;
					}
				}

				mout.debug(3) << "accept '" << it->first << "' [" << quantity << ']' << mout.endl;


				if (quantity.empty()){
					//drain::Logger mout("DataSet", __FUNCTION__);
					mout.info() << "quantities dataset:'" << datasetQuantity << "', data:'" << dataQuantity << "'"<< mout.endl;
					mout.warn() << "undefined quantity in " << it->first << ", using key=" << it->first << mout.endl;
					// Assign by path component "data3"
					dst.insert(typename map_t::value_type(it->first, DT(it->second, it->first)));
					//associate(dst, it->first, it->second);
				}
				else {
					if (dst.find(quantity) != dst.end()){ // already created
						//drain::Logger mout("DataSet", __FUNCTION__);
						mout.warn() << "quantity '" << quantity << "' replaced same quantity at " << it->first << mout.endl;
					}
					dst.insert(typename map_t::value_type(quantity, DT(it->second, quantity)));
					//typename datagroup_t::reverse_iterator rit = dst.rend();
					//mout.warn() << "last '" << "' [" << quantity << '=' << rit->first << ']' << rit->second << mout.endl;
					//dst[quantity] = T(it->second);
					//associate(dst, quantity, it->second);t.
				}
			}
		} // end pragma

		if (USE_REGEXP)
			mout.debug3() << "matched " << dst.size() << "(out of " << counter << ") data items with RegExp=/" << quantityRegExp.toStr() << '/' << mout.endl;
		else
			mout.debug3() << "collected " << dst.size() << " data items" << mout.endl;

		return t;
	};


	static
	//typename D::tree_t & adapt(typename D::tree_t & t, datagroup_t & dst, const datagroup_t & src){
	typename DT::tree_t & adapt(const datagroup_t & src, datagroup_t & dst){

		drain::Logger mout(__FUNCTION__, "DataGroup{" + ODIMPathElem::getKey(G)+"}");

		//drain::Logger mout("DataGroup." + ODIMPathElem::getKey(G), __FUNCTION__);
		// drain::Logger mout(__FUNCTION__, __FILE__);

		if (src.empty()){
			mout.debug(3) << "src empty" << mout.endl;
			return src.tree;
		}


		for (typename datagroup_t::const_iterator it=src.begin(); it!=src.end(); ++it){
			//dst.insert(typename map_t::value_type(it->first, D(it->second, it->first)));
			//dst.insert(typename map_t::value_type(it->first, D(it->second)));
			dst.insert(typename map_t::value_type(it->first, it->second));
		}
		mout.debug3() << "adapted " << dst.size() << " data items; " << src.begin()->first << "..." << mout.endl;

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
	inline
	const plaindata_t & getQualityData(const drain::RegExp & quantityRE) const {
		return this->quality.getData(quantityRE);
	}


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


// Well, needs quantity, primarily. So best place perhaps not here.
/**
 *  \tparam DT - data type (PolarSrc, PolarDst, CartesianSrc, CartesianDst, ...)
 */
/*
template <typename DT>  // PlainData<DT> & quality
void QualityDataSupport<DT>::createSimpleQualityData(PlainData<DT> & data, drain::image::Image & quality, double dataQuality, double undetectQuality, double nodataQuality) const {

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

	Image::iterator  it = data.data.begin();
	Image::iterator wit = quality.begin();
	while (it != data.data.end()){
		//if ((*it != odim.nodata) && (*it != odim.undetect))
		if (UNDETECT && (*it == odim.undetect))
			*wit = undetectCode;
		else if (NODATA && (*it == odim.nodata))
			*wit = nodataCode;
		else if (DATA)
			*wit = dataCode;
		++it;
		++wit;
	}

}
*/


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

		drain::Logger mout( __FUNCTION__, "Data<>");
		mout.warn() << "Swap (experimental fct)" << mout.endl;
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


	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataSet(typename data_t::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		datagroup_t(tree, quantityRegExp), QualityDataSupport<DT>(tree)
		{
	}

	DataSet(const DataSet<DT> & ds) : datagroup_t(ds), QualityDataSupport<DT>(ds) {
	}

	~DataSet(){

		drain::Logger mout(__FUNCTION__, "DataSet");

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

	}


	/// Retrieves data containing the given quantity. If not found, returns an empty array.
	/*
	inline
	const data_t & getData(const std::string & quantity) const {  // TODO: simply use original get()?
		return this->get(quantity);
	}

	/// Retrieves data containing the given quantity. If not found, creates an array.
	inline
	data_t & getData(const std::string & quantity) { // // TODO: simply use original get()?
		//drain::Logger mout("DataSetDst", __FUNCTION__);
		return this->get(quantity);
	}

	/// Retrieves data matching the given quantity. If not found, returns an empty array.
	inline
	const data_t & getData(const drain::RegExp & regExp) const { // TODO: simply use original get()?
		return this->get(regExp);
	}
	*/

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
		//odim.copyToDataSet(this->tree);
		//if (!DataTools::removeIfNoSave(this->tree))
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, this->tree);
		DataTools::updateInternalAttributes(this->tree); // TEST2019/09 // images, including DataSet.data, TODO: skip children
		//DataTools::updateInternalAttributes(this->tree, drain::FlexVariableMap()); // TEST2019/09 // images, including DataSet.data, TODO: skip children
	}

	// TODO: consider this to destructor!
	inline
	void updateTree3(const typename DT::odim_t & odim) const {  //
		std::cout << "updateTree3 const \n";
		//ODIM::copyToH5<ODIMPathElem::DATASET>(odim, tree);
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
template <typename DT>
class DataSetMap : public std::map<double, DataSet<DT> > {
};


template <typename DT>
class DataSetList : public std::list<DataSet<DT> > {
};




} // rack::

#endif /* DATA_H_ */

