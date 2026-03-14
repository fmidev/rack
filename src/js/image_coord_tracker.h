/** Automatically wrapped from " src/js/image_coord_tracker.js " */

const char* image_coord_tracker = R"JS(/*  src/js/image_coord_tracker.js  */
/// Add mouse listerners 
function image_coord_tracker(){

    const elems = document.querySelectorAll(".MOUSE");

    elems.forEach(panel => {

	//panel.selectorRect = panel.querySelector(".SELECT");

	// Mouse tracker
	var plane_id = panel.getAttribute('data-mouse-plane');

	if (plane_id){
	    console.warn(plane_id);
	}
	else {
	    console.error("Mouse listener not found")
	}
	// Actaually "only" one level higher
	const plane = document.querySelector('#'+plane_id);
	if (plane){
	    console.log(plane);
	}
	else {
	    console.error("Mouse listener element #"+plane_id+" not found")
	}

	
	// TODO: direct attr in frame
	var metadata  = panel.querySelector(".SHARED");
	var epsg = null;
	if (metadata.hasAttribute('EPSG')){
	    epsg = metadata.getAttribute('EPSG');
	    console.info('EPSG='+epsg)
	}

	
	
	const BBOX_KEY='data-bbox';
	var bbox = [0,0,1,1];
	var frame   = panel.querySelector(".IMAGE_BORDER");
	if (frame.hasAttribute(BBOX_KEY)){
	    bbox = frame.getAttribute(BBOX_KEY).split(',');
	    console.log("data-bbox: ", bbox);
	}
	// TEST! So it was only for meta data?
	frame = plane


	// const frameBbox  = frame.getBoundingClientRect();

	var monitor = null;

	
	if (monitor = panel.querySelector('.MONITOR')){
	    const m = new CoordMonitor(panel, frame, monitor, bbox);
	    plane.addEventListener("mousemove", (ev) => {
		m.updateMove(ev.clientX, ev.clientY);
	    })
	}

	if (monitor = panel.querySelector('.MONITOR_DOWN')){
	    const m = new CoordMonitor(panel, frame, monitor, bbox);
	    plane.addEventListener("mousedown", (ev) => {
		m.updateDown(ev.clientX, ev.clientY);
	    })
	}
	if (monitor = panel.querySelector('.MONITOR_UP')){
	    const m = new CoordMonitor(panel, frame, monitor, bbox);
	    plane.addEventListener("mouseup", (ev) => {
		m.updateUp(ev.clientX, ev.clientY);
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

function CoordMonitor(group, frame, monitor, bbox){
    this.group = group;
    window.superGroup = group;
    this.bboxFrame = frame.getBoundingClientRect();
    this.monitor = monitor;
    this.bboxGeo    = new BBox(bbox);
}

CoordMonitor.prototype.getRelativeX = function(x){
    return (x - this.bboxFrame.left)/this.bboxFrame.width;
}

CoordMonitor.prototype.getRelativeY = function(y){
    return (y - this.bboxFrame.top)/this.bboxFrame.height;
}

CoordMonitor.prototype.getLongitude = function(xRel){
    return this.bboxGeo.left + xRel*this.bboxGeo.width;
}

CoordMonitor.prototype.getLatitude = function(yRel){
    return this.bboxGeo.top + (1.0-yRel)*this.bboxGeo.height;
}


/*
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
*/



/// Default update function
CoordMonitor.prototype.update = function(x,y){
    var a = [];
    a.push(x);
    a.push(y);
    var trans = a.join(',');
    //this.monitor.setAttribute("chloroform",`translate(${trans})`);
    var xRel = this.getRelativeX(x); //this.rescaleX(x, this.bboxFrame);
    var yRel = this.getRelativeY(y); //1.0-this.rescaleY(y,this.bboxFrame);
    //a.push(xRel);
    //a.push(yRel);
    // x = Math.round(this.bboxGeo.xLL + this.bboxGeo.width * x);
    // y = Math.round(this.bboxGeo.yLL + this.bboxGeo.height* y);
    // x = Math.round(this.descaleX(x, this.bboxGeo));
    // y = Math.round(this.descaleY(y, this.bboxGeo));
    x = Math.round(this.getLongitude(xRel));
    y = Math.round(this.getLatitude(yRel));
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
   TODO: base-update for those above.
   And then store start (upon down) and end (upon up) in image coords 
 */
CoordMonitor.prototype.updateMove = CoordMonitor.prototype.update
CoordMonitor.prototype.updateDown = CoordMonitor.prototype.update
CoordMonitor.prototype.updateUp   = CoordMonitor.prototype.update
)JS";
