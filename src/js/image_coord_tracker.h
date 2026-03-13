/** Automatically wrapped from " src/js/image_coord_tracker.js " */

const char* image_coord_tracker = R"JS(/*  src/js/image_coord_tracker.js  */
/// Add mouse listerners 
function image_coord_tracker(){

    const elems = document.querySelectorAll(".MOUSE");

    elems.forEach(panel => {

	const frame   = panel.querySelector(".IMAGE_BORDER");

	// TODO: direct attr in frame
	var metadata  = panel.querySelector(".SHARED");
	var epsg = null;
	if (metadata.hasAttribute('EPSG')){
	    epsg = metadata.getAttribute('EPSG');
	    console.info('EPSG='+epsg)
	}

	const BBOX_KEY='data-bbox';

	var bbox = [0,0,1,1];
	if (frame.hasAttribute(BBOX_KEY)){
	    bbox = frame.getAttribute(BBOX_KEY).split(',');
	}
	console.log(bbox);

	const frameBbox  = frame.getBoundingClientRect();

	var monitor = null;

	if (monitor = panel.querySelector('.MONITOR')){
	    const m = new CoordMonitor(frame, monitor, bbox);
	    frame.addEventListener("mousemove", (ev) => {
		m.updateMove(ev.clientX, ev.clientY);
		//m.updateMove((ev.clientX - frameBbox.left)/frameBbox.width, (ev.clientY - frameBbox.top)/frameBbox.height)
	    })
	}

	if (monitor = panel.querySelector('.MONITOR_DOWN')){
	    const m = new CoordMonitor(frame, monitor, bbox);
	    frame.addEventListener("mousedown", (ev) => {
		m.updateDown(ev.clientX, ev.clientY);
		//((ev.clientX - frameBbox.left)/frameBbox.width, (ev.clientY - frameBbox.top)/frameBbox.height)
	    })
	}
	if (monitor = panel.querySelector('.MONITOR_UP')){
	    const m = new CoordMonitor(frame, monitor, bbox);
	    frame.addEventListener("mouseup", (ev) => {
		m.updateUp(ev.clientX, ev.clientY);
		//((ev.clientX - frameBbox.left)/frameBbox.width, (ev.clientY - frameBbox.top)/frameBbox.height)
	    })
	}


    })
}

function BBox(bbox){
    /*
    this.xLL = parseFloat(bbox[0]);
    this.yLL = parseFloat(bbox[1]);
    this.xUR = parseFloat(bbox[2]);
    this.yUR = parseFloat(bbox[3]);
    this.width  = this.xUR-this.xLL;
    this.height = this.yLL-this.yUR;
    */
    this.left = parseFloat(bbox[0]);
    this.top  = parseFloat(bbox[3]);
    this.width  = parseFloat(bbox[2]) - this.left;
    this.height = parseFloat(bbox[1]) - this.top;
    
}

function CoordMonitor(frame, monitor, bbox){
    this.bboxFrame = frame.getBoundingClientRect();
    this.monitor = monitor;
    this.bboxGeo    = new BBox(bbox);
}

CoordMonitor.prototype.rescaleX = function(x, bbox){
    return (x - bbox.left)/bbox.width;
}

CoordMonitor.prototype.rescaleY = function(y, bbox){
    return (y - bbox.top)/bbox.height;
}

CoordMonitor.prototype.rescaleYinv = function(y, bbox){
    return (bbox.top - y)/bbox.height;
}


CoordMonitor.prototype.descaleX = function(x, bbox){
    return bbox.left + bbox.width*x;
}

CoordMonitor.prototype.descaleY = function(y, bbox){
    return bbox.top + bbox.height*y;
}

CoordMonitor.prototype.descaleYinv = function(y, bbox){
    return bbox.top + (1.0-y)*bbox.height;
}


/*
  CoordMonitor.prototype.getRelativeX = function(x){
  return (x - this.bboxFrame.left)/this.bboxFrame.width
  }

  CoordMonitor.prototype.getRelativeY = function(y){
  return (y - this.bboxFrame.top)/this.bboxFrame.height
  }
*/


/// Default update function
CoordMonitor.prototype.update = function(x,y){
    var a = [];
    a.push(x);
    a.push(y);
    x = this.rescaleX(x, this.bboxFrame);
    y = 1.0-this.rescaleY(y,this.bboxFrame);
    a.push(x);
    a.push(y);
    // x = Math.round(this.bboxGeo.xLL + this.bboxGeo.width * x);
    // y = Math.round(this.bboxGeo.yLL + this.bboxGeo.height* y);
    x = Math.round(this.descale(x, this.bboxGeo));
    y = Math.round(this.descale(y, this.bboxGeo));
    // x = this.rescaleX(x, this.bboxGeo);
    // y = this.rescaleYinv(y, this.bboxGeo);
    a.push(x);
    a.push(y);
    // a.push(Math.round(this.bboxGeo.xLL + this.bboxGeo.width * x));
    // a.push(Math.round(this.bboxGeo.yLL + this.bboxGeo.height* y));
    this.monitor.textContent = a.join(',')
    //this.monitor.textContent = ''+Math.round(this.bboxGeo.xLL + x*this.bboxGeo.width)+','+Math.round(this.bboxGeo.yLL + (1.0-y)*this.bboxGeo.height)
}
/// Default actions. Can be overridden by C++ code XML::JAVASCRIPT_SCOPE (see below)
/**
   Available "context variables":
   this.bboxFrame: SVG element BBOX rect
   this.bboxGeo: BBox object
 */
CoordMonitor.prototype.updateMove = CoordMonitor.prototype.update
CoordMonitor.prototype.updateDown = CoordMonitor.prototype.update
CoordMonitor.prototype.updateUp   = CoordMonitor.prototype.update
)JS";
