/** Automatically wrapped from " src/js/image_value_tracker.js " */

const char* image_value_tracker = R"JS(<![CDATA[
function Encoding(encoding){
    this.gain = 1.0;
    this.offset = 0.0;

    if (!encoding)
	return;
    
    if (typeof(encoding) === "string"){
	//console.info(elem);
	encoding = encoding.split(',');
	switch (encoding.length){
	case 4:
	case 3:
	case 2:
	    this.offset = parseFloat(encoding[1]);
	case 1:
	    this.gain   = parseFloat(encoding[0]);
	    break;
	default:
	    console.warn("data has empty 'encoding' attribute:");
	    //console.warn(elem);
	}
	console.info('scaling: ', this.gain, ',', this.offset);
    }

}

async function set_image_value_tracker(imgElem, encoding, monitorElem){

    await imgElem.decode();
    const bbox  = imgElem.getBoundingClientRect();
    console.warn('bbox: ', bbox);
    // const w = imgElem.naturalWidth  | 1;
    // const h = imgElem.naturalHeight | 1;
    const w = bbox.width;
    const h = bbox.height;
    const canvas = new OffscreenCanvas(w, h);
    console.info(canvas)
    
    const ctx = canvas.getContext("2d", { willReadFrequently: true });
    
    // Expose for debugging
    window.rackster = { imgElem, w, h, canvas, ctx };
    
    // Draw and read
    ctx.clearRect(0, 0, w, h);
    ctx.drawImage(imgElem, 0, 0);

    
    // critical:
    const imageData = ctx.getImageData(0, 0, w, h);
    const data = imageData.data; 

    function grayAt(x, y) {
	const gain   = encoding.gain;
	const offset = encoding.offset;
	
	if (x < 0 || y < 0 || x >= w || y >= h) return "";
	const i = (y * w + x) * 4;
	return (gain*(data[i] + (data[i+1]<<8)) + offset).toFixed(2);
    }

    imgElem.addEventListener("mousemove", (ev) => {
	const x = ev.clientX - bbox.left;
	const y = h - (ev.clientY - bbox.top);
	// console.info(grayAt(x, y))
	// const i = x / bbox.width;
	// const j = y / bbox.height;
	// func(i,j)`The formatted number is ${number.toFixed(2)}`
	// var value = grayAt(x,y);
	// monitorElem.textContent = `...formatted ${value.toFixed(2)}`
	monitorElem.textContent = '('+x+','+y+'): ' + grayAt(x,y); //.toFixed(2);
    })
    
    window.rackdata = data

}

function image_value_tracker(){

    // const elems = document.querySelectorAll("metadata[data-base64]");
    const elems = document.querySelectorAll(".MOUSE_VALUE");

    elems.forEach(elem => {
	// set_image_value_tracker(elem)
	const dataElem = elem.querySelector(".MOUSE_VALUE_DATA");
	window.metadata = dataElem;
	var encoding = new Encoding(dataElem.getAttribute("data-encoding"))

	const monitorElem = elem.querySelector(".MOUSE_VALUE_MONITOR");
	
	set_image_value_tracker(dataElem, encoding, monitorElem);
	var type = dataElem.getAttribute("data-basetype");
	
	
    });
    
}
//]]>)JS";
