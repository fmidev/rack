function RadarDataEncoding(encoding){

    this.type   = "";
    this.scale  = 1.0;
    this.offset = 0.0;
    this.nodata = null;
    this.undetect = null;

    // Could be hidden, for now
    this.precision = 2; // digits

    if (!encoding)
	return;
    
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


async function set_image_value_tracker(imgElem, encoding, coordMonitorElem, valueMonitorElem){

    const svg = document.querySelector("svg");
    
    await imgElem.decode();
    const bbox  = imgElem.getBoundingClientRect();
    console.log('bbox: ', bbox);
    // const w = imgElem.naturalWidth  | 1;
    // const h = imgElem.naturalHeight | 1;
    const w = Math.round(bbox.width);
    const h = Math.round(bbox.height);
    const canvas = new OffscreenCanvas(w, h);
    console.info(canvas)
    
    const ctx = canvas.getContext("2d", { willReadFrequently: true });
    
    // Expose for debugging
    // window.rackster = { imgElem, w, h, canvas, ctx };
    
    // Draw and read
    ctx.clearRect(0, 0, w, h);
    // ctx.drawImage(imgElem, 0, 0);
    ctx.drawImage(imgElem, 0, 0, w, h);

    
    
    // critical:
    const imageData = ctx.getImageData(0, 0, w, h);
    const data = imageData.data; 

    // console.warn(imageData)
    
    function grayAt(x, y) {

	const e = encoding;

	//x = Math.floor(x)
	//y = Math.floor(y)
	
	if (x < 0 || y < 0 || x >= w || y >= h) return "";
	const i = (y * w + x) * 4;
	// Red: higher bits, Green: lower bits
	const v = (data[i]<<8) + data[i+1];
	// return ""+(data[i])+'+'+data[i+1] + " = " + e.decode(v);
	return e.decode(v);

    }

    imgElem.addEventListener("mousemove", (ev) => {

	// svg - doc root linked above
	/*
	// This worked badly, if browser zoom applied.
	const pt = svg.createSVGPoint();
	pt.x = ev.clientX;
	pt.y = ev.clientY;
	const p = pt.matrixTransform(imgElem.getScreenCTM().inverse());
	const x = Math.floor(p.x);
	const y = Math.floor(p.y);
	*/
	var my_bbox  = imgElem.getBoundingClientRect();
	var x = Math.floor(ev.clientX - my_bbox.left);
	var y = Math.floor(ev.clientY - my_bbox.top);
	coordMonitorElem.textContent = '('+x+','+y+')'; //.toFixed(2);
	valueMonitorElem.textContent = grayAt(x,y); //.toFixed(2);
    })
    
    // window.rackdata = data

}

function image_value_tracker(){

    // const elems = document.querySelectorAll("metadata[data-base64]");
    const elems = document.querySelectorAll(".MOUSE_VALUE");

    elems.forEach(elem => {

	const dataElem = elem.querySelector(".MOUSE_VALUE_DATA");
	window.metadata = dataElem;
	var encoding = new RadarDataEncoding(dataElem.getAttribute("data-encoding"))

	const coordMonitorElem = elem.querySelector(".COORD_MONITOR");
	const valueMonitorElem = elem.querySelector(".VALUE_MONITOR");
	
	set_image_value_tracker(dataElem, encoding, coordMonitorElem, valueMonitorElem);
	// var type = dataElem.getAttribute("data-basetype");
	
    });
    
}
