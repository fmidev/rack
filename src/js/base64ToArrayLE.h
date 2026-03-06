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
//]]>)JS";
