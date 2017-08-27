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
#include <drain/util/Path.h>

#include "ODIM.h"
#include "data/VerticalODIM.h"
//#include "product/DataConversionOp.h"

#include "DataSelector.h"

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
 *  \tparam T  - tree type
 *  \tparam TI - tree iterator: iterator for non-const, const_iterator for const
 *  \tparam D  - tree type
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



/// Tool for selecting datasets based on paths, quantities and min/max elevations.


/**
 *  The template parameters are expected to be exactly the following, but all of them const or non-const:
 *  \tparam T - HDF5 tree, as HI5TREE
 *  \tparam D - two-dimensional data, as drain::image::Image
 *  \tparam M - metadata, as rack::PolarODIM
 */
template <typename DT>
class PlainData {
public:

	typedef typename DT::tree_t tree_t;
	typedef typename DT::image_t image_t;
	typedef typename DT::odim_t odim_t;

	/// Constructor referring to HDF5 structure
	PlainData(tree_t & tree) :
			tree(tree),
			data(tree["data"].data.dataSet),
			odim(data)
	{
	}

	/// Copy constructor, also for referencing non-const as const.
	/**
	 *   Compiler returns error if odim types ar incompatible.
	 */
	template <typename DT2>
	PlainData(const PlainData<DT2> & d) :
				tree(d.tree),
				data(tree["data"].data.dataSet),
				odim(data)
	{
	}

	/// Saves type and sets the type of the actual data array as well.
	template <class T>
	inline
	void setTypeDefaults(const T & type, const std::string & values = ""){
		odim.setTypeDefaults(type, values);
		data.setType(type);
	}

	/// Sets dimensions of data array and metadata.
	inline
	void setGeometry(size_t cols, size_t rows){
		odim.setGeometry(cols, rows);
		data.setGeometry(cols, rows);
	}


	tree_t & tree;

	image_t & data;

	// Metadata structure
	odim_t odim;

	/*
	inline
	image_t & getPalette(){
		return tree["palette"].data.dataSet;
	}
	*/

	inline
	void updateTree(){
		odim.copyToData(tree);
	}

protected:


	// mutable Image palette;

};



template <typename DT>
inline
std::ostream & operator<<(std::ostream & ostr, const PlainData<DT> & d){
	ostr << d.data << ", ODIM:\t ";
	ostr << d.odim << '\n';
	ostr << d.data.properties << '\n';
	return ostr;
}






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
class Data : public PlainData<DT> {
public:

	typedef PlainData<DT> plaindata_t;

	Data(typename DT::tree_t & tree) : PlainData<DT>(tree){
	}

	// Data(const HI5TREE & src, const std::string & quantity = "^DBZH$");
	virtual ~Data(){};

	/// Finds local quality data; maybe empty.
	/*
	 *  Returns local, "own" quality. Ie. if data is under data[i], returns data[i]/quality[j] for which quantity=[quantity]
	 *  \param quantity - qua
	 *  \param tmp - create temporary data array that will not be stored by Hi5Writer().
	 */
	plaindata_t & getQualityData(const std::string & quantity = "QIND") const { // , bool tmp = false
		return getQualityData(qualityDataMap, quantity, this->tree);  // , tmp
	}




	static
	void findExistingQualityPath(const std::string & quantity, const typename plaindata_t::tree_t & tree, std::string & path){
		// "/quality.." =>
		// "^/quality.." =>
		DataSelector selector("^/quality[0-9]+$", std::string("^")+quantity+std::string("$"));
		DataSelector::getPath(tree, selector, path);
	}

	static
	bool hasQualityData(std::map<std::string, plaindata_t > & cacheMap, const std::string & quantity, typename plaindata_t::tree_t & tree){

		if (cacheMap.find(quantity) != cacheMap.end())
			return true;
		else {
			std::string path = "";
			findExistingQualityPath(quantity, tree, path);
			return !path.empty();
		}

	}


	static
	plaindata_t & getQualityData(std::map<std::string, plaindata_t > & cacheMap, const std::string & quantity, typename plaindata_t::tree_t & tree){ //  , bool tmp = false
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
			//DataSelector selector("^/quality[0-9]+$", std::string("^")+quantity+std::string("$"));
			//DataSelector::getPath(tree, selector, path);
			if (path.empty()){  // create it
				// "/quality.." =>
				// "^/quality.." =>
				path = "quality1";
				DataSelector::getNextOrdinalPath(tree, "^/?quality[0-9]+$", path);
				// std::cerr << "PlainData::getQualityData(): creating '"<< quantity << "' in '"<< path <<"', caching" << std::endl;
			}

			it = cacheMap.insert(cacheMap.begin(), typename std::map<std::string, plaindata_t >::value_type(quantity, plaindata_t(tree(path))));  // WAS [path]
			// it = cacheMap.insert(cacheMap.begin(), typename plaindata_t(quantity, plaindata_t(tree[path])));

			return it->second;
		}
	}


	/// Returns true if non-empty, associated QIND data exists.
	inline
	bool hasQuality(const std::string & quantity = "QIND") const {
		return hasQualityData(qualityDataMap, quantity, this->tree);
		//return !getQualityData(quantity).data.isEmpty();
	};
	// static 	PlainData<T,D,M> & getQualityField(T & tree);

	// TODO
	// T & tree;
	// T & parentTree;

protected:

	mutable
	std::map<std::string, plaindata_t > qualityDataMap;



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
class DataSet : public std::map<std::string, Data<DT> > { // typename T::data_t
public:

	typedef std::map<std::string, Data<DT> > map_type;

	/// Given a \c dataset subtree, like tree["dataset3"], constructs a data map of desired quantities.
	DataSet(typename DT::tree_t & tree, const drain::RegExp & quantityRegExp = drain::RegExp()) :
		tree(init(tree, quantityRegExp, *this)) {
	}

	typename DT::tree_t & tree;  // DON'T HACK. TO BE MOVED protected FOR SUBCLASSES?


protected:


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
				drain::MonitorSource mout("DataSet", __FUNCTION__);
				mout.warn() << "quantities dataset:'" << datasetQuantity << "', data:'" << dataQuantity << "'"<< mout.endl;
				mout.warn() << "undefined quantity in " << it->first << ", using key=" << it->first << mout.endl;
				// Assign by path component "data3"
				dst.insert(typename map_type::value_type(it->first, Data<DT>(it->second)));
				//associate(dst, it->first, it->second);
			}
			else if (quantityRegExp.test(quantity) ){
				if (dst.find(quantity) != dst.end()){ // already created
					drain::MonitorSource mout("DataSet", __FUNCTION__);
					mout.warn() << "quantity '" << quantity << "' replaced same quantity at " << it->first << mout.endl;
				}
				dst.insert(typename map_type::value_type(quantity, Data<DT>(it->second)));
				//dst[quantity] = T(it->second);
				//associate(dst, quantity, it->second);
			}
			else if ((!quantity.empty() && (quantity.at(0) == '~') && quantityRegExp.test(quantity.substr(1)))){ // normalized  "~QUANTITY" experimental (scaled data standard)
				dst.insert(typename map_type::value_type(quantity, Data<DT>(it->second)));
				//associate(dst, quantity, it->second);
			}
		}

		return datasetTree;
	};

};




/// A map containing the data arrays of a sweep; possibly several quantities DBZH, VRAD, ...
/**
 *
 */
template <typename S = PolarSrc>
class DataSetSrc : public DataSet<S> {

public:

	//typedef Data<HI5TREE const, Image const, M> data_t;

	inline
	DataSetSrc(const HI5TREE & datasetGroup, const drain::RegExp & quantityRegExp = drain::RegExp()) : DataSet<S>(datasetGroup, quantityRegExp) {  // , typename S::tree_t::const_iterator
	}

	const Data<S> & getFirstData() const {

		drain::MonitorSource mout("DataSetSrc", __FUNCTION__);

		if (this->begin() == this->end())
			mout.error() << "no data" << mout.endl;

		return this->begin()->second;

	}

	const Data<S> & getData(const std::string & quantityKey) const {
		const DataSetSrc & src = *this;
		if (src.find(quantityKey) != src.end()){
			return src.find(quantityKey)->second;
				}
			else	 {
				static typename Data<S>::tree_t t;
				static Data<S> empty(t);
				return empty;
			}
	}

	// experimental
	// const drain::VariableMap & whatTEST;

	///
	/**
	 *  \param quantity -
	 *  \param tmp -
	 */
	PlainData<S> & getQualityData(const std::string & qualityQuantity = "QIND", const std::string & quantity = "") const {

		if (quantity.empty()) // GLOBAL DATA
			return Data<S>::getQualityData(qualityDataMap, qualityQuantity, this->tree);
		else {
			// if !hasData(quantity)
			return getData(quantity).getQualityData(qualityQuantity);
		}

	}

private:

	//DataSet<PlainDataDst, PlainData<PolarDst>::tree_t::iterator> qualityDataMap;
	mutable
	std::map<std::string, PlainData<S> > qualityDataMap;

};



/// A map of radar data (of a sweep or polar product), indexed by quantity code.
template <typename D = PolarDst>
class  DataSetDst : public DataSet<D> { //

public:

	inline
	DataSetDst(HI5TREE & dst, const drain::RegExp & quantityRegExp = drain::RegExp()) :  DataSet<D>(dst, quantityRegExp) { // , typename D::tree_t::iterator

	};

	typedef Data<D> data_t;



	Data<D> & getFirstData() {


		if (this->begin() == this->end()){
			drain::MonitorSource mout("DataSetDst", __FUNCTION__);
			mout.error() << "no data" << mout.endl;
		}

		return this->begin()->second;

	}

	Data<D> & getData(const std::string & quantityKey) { //

		drain::MonitorSource mout("DataSetDst", __FUNCTION__);

		//DataSetDst<D> & dst = *this;

		typename DataSetDst<D>::iterator it = this->find(quantityKey);

		if (it != this->end()){
			return it->second;
		}
		else {
			if (quantityKey.empty()){
				mout.warn() << " quantityKey empty" << mout.endl;
			}

			std::string dataPath = "data1";  // hence finally "data<n>" //// or "quality<1>"

			//DataSelector::getNextOrdinalPath(this->tree, "data[0-9]+/[^0-9]+$", dataPath);
			DataSelector::getNextOrdinalPath(this->tree, "data[0-9]+/?$", dataPath);

			typename D::tree_t & t = this->tree(dataPath);

			it = this->insert(this->begin(), std::pair<std::string, Data<D> >(quantityKey, Data<D>(t)));
			t["what"].data.attributes["quantity"] = quantityKey;

			if (it->second.odim.quantity.empty()){
				it->second.odim.quantity = quantityKey;
			}
			/*
			if (!it->second.odim.hasKey("quantity")){
				mout.warn() << "asked for " << quantityKey << ", but odim" << it->second.odim << mout.endl;
				//it->second.odim; //.quantity = quantity;
			}
			*/
			//it->second.odim.gain;
			//return this->find(quantityKey)->second;
			// TODO: it->second.odim.quantity = quantity ?
			return it->second;
		}

	}



	PlainData<D> & getQualityData(const std::string & qualityQuantity = "QIND", const std::string & quantity = "") {  // raise / join with src

		//drain::MonitorSource mout("DataSetDst", __FUNCTION__);
		if (quantity.empty()) // GLOBAL DATA
			return Data<D>::getQualityData(qualityDataMap, qualityQuantity, this->tree);
		else {
			// if !hasData(quantity)
			return getData(quantity).getQualityData(qualityQuantity);
		}
		//return Data<D>::getQualityData(qualityDataMap, quantity, this->tree);
	}

	template <class T>
	inline
	void updateTree(const T & odim){  //
		odim.copyToDataSet(this->tree);
	}


protected:

	//DataSet<PlainDataDst, PlainData<PolarDst>::tree_t::iterator> qualityDataMap;
	mutable
	std::map<std::string, typename Data<D>::plaindata_t > qualityDataMap;

};

//#define ProductDst DataSetDst<DataDst> // TMP FIX

/// Structure for storing sweeps by their elevation angle.
/**  Note: this approach fails if a volume contains several azimuthal sweeps with a same elevation angle.
 *
 */

//typedef std::map<double, DataSetSrc<PolarSrc> >  DataSetSrcMap;

//TODO template <typename S = PolarSrc >
class DataSetSrcMap : public std::map<double, DataSetSrc<PolarSrc> > {

};

//#define SweepMapSrc DataSetSrcMap  // TMP FIX


template <typename D=PolarDst>
class DataSetDstMap : public std::map<double, DataSetDst<D> > {

public:

	typedef std::pair<double, DataSetDst<D> > value_type;

	DataSetDstMap(){};

	// Pics all the data fields but not (quality fields).
	DataSetDstMap(HI5TREE & src, const DataSelector & selector = DataSelector("data[0-9]/?$")){

		std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
		DataSelector::getPaths(src,  selector, dataPaths);

		for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

			const std::string parent = DataSelector::getParent(*it);
			HI5TREE & s = src(parent);
			if (s.hasChild("where")){
				const drain::VariableMap & a = s["where"].data.attributes;
				if (a.hasKey("elangle")){
					const double elangle = a["elangle"];
					if (this->find(elangle) == this->end()){
						//mout.debug(2) << "add "  << elangle << ':'  << parent << mout.endl;
						this->insert( value_type(elangle, DataSetDst<D>(s, drain::RegExp(selector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
					}
				}

			}
		}
	};

};

//typedef std::map<double, DataSetDst<PolarDst> >  DataSetDstMap;



} // rack::

#endif /* DATA_H_ */

// Rack
