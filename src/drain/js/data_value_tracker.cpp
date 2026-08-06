/** Automatically generated from " src/drain/js/data_value_tracker.js " */
namespace javascript {
const char *data_value_tracker = R"JS(

// NEW2, pruned!

/// Attaches an IMAGE element two special objects: data array and encoding.
/**
   Essentially, data array is a converted copy of the image data.
   Encoding is defined externally, by the data provider.
   
  */
async function init_data_elem(dataElem, encoding){

    await dataElem.decode();
    const bbox  = dataElem.getBoundingClientRect();
    console.log('bbox: ', bbox);
    // const w = imgElem.naturalWidth  | 1;
    // const h = imgElem.naturalHeight | 1;
    const w = Math.round(bbox.width);
    const h = Math.round(bbox.height);
    const canvas = new OffscreenCanvas(w, h);
    // console.info(canvas)
    
    const ctx = canvas.getContext("2d", { willReadFrequently: true });

    // Draw and read
    ctx.clearRect(0, 0, w, h);
    // ctx.drawImage(imgElem, 0, 0);
    ctx.drawImage(dataElem, 0, 0, w, h);

    // critical:
    const imageData = ctx.getImageData(0, 0, w, h);
    // const data = imageData.data; 
 
    // finally, store it under the elem
    dataElem.data = imageData.data; 
    dataElem.data.w = w;
    dataElem.data.h = h;
    // dataElem.encoding = new RadarDataEncoding(dataElem.getAttribute("data-encoding"))

    
    if (encoding){
	if (dataElem.encoding){
	    console.warn("replacing dataElem.encoding with", encoding, " current: ");
	    console.warn(encoding);
	}
	dataElem.encoding = encoding;
    }
    else {
	dataElem.encoding = {decode: function(value){return value}};
    }

}

// New
function getDataValue(dataElem, x, y) {

	// IMPORTANT: x and y must be int values
    if (x < 0 || y < 0 || x >= dataElem.data.w || y >= dataElem.data.h) return "-";

    const i = (y * dataElem.data.w + x) * 4;
    // Red: higher bits, Green: lower bits
    const v = (dataElem.data[i]<<8) + dataElem.data[i+1];
    // return ""+(data[i])+'+'+data[i+1] + " = " + e.decode(v);
    // console.info(dataElem.encoding)
    return dataElem.encoding.decode(v);
    
}

)JS";
} // javascript::

