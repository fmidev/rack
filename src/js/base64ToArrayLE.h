/** Automatically wrapped from " src/js/base64ToArrayLE.js " */

const char* base64ToArrayLE = R"JS(<![CDATA[
/**
 *  \param b64:string - base64 data 
 *  \param ArrayCls   - Array class, like Float32Array
 */
function base64ToArrayLE(b64, ArrayCls) {

    const bin = atob(b64);
    const bytes = new Uint8Array(bin.length);
    for (let i = 0; i < bin.length; i++){
	bytes[i] = bin.charCodeAt(i);
    }

    if (!ArrayCls){
	ArrayCls = Float32Array;
    }
    
    // Intermediate data container. 100% safe endianness 
    const dv = new DataView(bytes.buffer);

    // Resulting array (of type Float32Array, for example)
    const n = bytes.byteLength / ArrayCls.BYTES_PER_ELEMENT;
    const out = new ArrayCls(n);

    /// Construct and link getter, like getFloat32()
    var getFuncName = 'get'+ArrayCls.name.replace('Array','')
    dv.getFunc = dv[getFuncName]
    
    for (let i = 0; i < n; i++){
	out[i] = dv.getFunc(i * ArrayCls.BYTES_PER_ELEMENT, true); // little-endian
    }

    return out;
}

function demo_base64(){

    const elems = document.querySelectorAll("metadata[data-base64]");

    elems.forEach(elem => {
	// set_image_value_tracker(elem)
	console.info(elem)
	window.metadata = elem
	var b64 = elem.getAttribute("data-base64")
	console.info(b64)
	var type = elem.getAttribute("data-basetype")
	
	switch (type){
	case 'int8':
	    type = Int8Array;
	    break;
	case 'uint8':
	    type = Uint8Array;
	    break;
	//case 'Uint8Clamped':
	//   type = Uint8ClampedArray;
	//   break;
	case 'int16':
	case 'short':
	    type = Int16Array;
	    break;
	case 'uint16':
	case 'unsigned short':
	    type = Uint16Array;
	    break;
	case 'int32':
	    type = Int32Array;
	    break;
	case 'uint32':
	    type = Uint32Array;
	    break;
	/*
	case 'bigInt64':
	    type = BigInt64Array;
	    break;
	case 'bigUint64':
	    type = BigUint64Array;
	    break;
	*/
	case 'float16':
	    type = Float16Array;
	    break;
	case 'float': // ?x
	case 'float32':
	    type = Float32Array;
	    break;
	default:
	    console.warn('base64 unsupported type:', type)
	    type = null;
	}
	var arr = base64ToArrayLE(b64, type)
	//base64ToFloat32ArrayLE(b64)
	console.info(arr)
    })
}
//]]>)JS";
