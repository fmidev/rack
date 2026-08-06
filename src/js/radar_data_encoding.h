/** Automatically generated from " src/js/radar_data_encoding.js " */

#ifndef JS_INCLUDE_radar_data_encoding
#define JS_INCLUDE_radar_data_encoding


namespace javascript {
const char* radar_data_encoding = R"JS(// Source:  src/js/radar_data_encoding.js

/**
   "Implements DataEncoding.decode(value)"
 */

function RadarDataEncoding(encoding){

    this.type   = "";
    this.scale  = 1.0;
    this.offset = 0.0;
    this.nodata = null;
    this.undetect = null;

    // Could be hidden, for now
    this.precision = 2; // digits

    if (!encoding){
	// Error?
	return;
    }
    
    if (typeof(encoding) === "string"){
	// console.info(elem);
	encoding = encoding.split(',');
	switch (encoding.length){
	case 5:
	    this.undetect = parseFloat(encoding[4]);
	case 4:
	    this.nodata   = parseFloat(encoding[3]);
	case 3:
	    this.offset   = parseFloat(encoding[2]);
	case 2:
	    this.scale    = parseFloat(encoding[1]);
	case 1:
	    this.type     = encoding[0];
	    break;
	default:
	    console.warn("data has extra 'encoding' attributes: ", encoding);
	    //console.warn(elem);
	}
	console.info('encoding: ' + this);
    }

}

RadarDataEncoding.prototype.toString = function(){ return `${this.type},${this.scale},${this.offset},${this.nodata},${this.undetect}`}

RadarDataEncoding.prototype.decode = function(value){

    if (value === this.nodata){
	return 'nodata';
    }
    else if (value === this.undetect){
	return 'undetect';
    }
    else {
	return (this.scale*value + this.offset).toFixed(this.precision);
    }
    
}
)JS";
} // javascript::
#endif
