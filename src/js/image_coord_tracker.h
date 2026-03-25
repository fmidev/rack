/** Automatically wrapped from " src/js/image_coord_tracker.js " */

const char* image_coord_tracker = R"JS(/*  src/js/image_coord_tracker.js  */
/// Add mouse listerners 
function image_coord_tracker(){

    const elems = document.querySelectorAll(".MOUSE");

    elems.forEach(panel => {


	// Plane listening to mouse events
	const coordTracker = panel.querySelector(".MOUSE_TRACKER");
	if (!coordTracker){
	    console.info(panel);
	    console.error("elem .MOUSE found without child .MOUSE_TRACKER");
	    return;
	}

	// var precision = parseInt(coordTracker.getAttribute("data-precision")); // for rect
	
	// TODO: direct attr in frame
	var epsg = null;
	var metadata  = panel.querySelector("metadata.SHARED");
	if (metadata.hasAttribute('EPSG')){
	    epsg = metadata.getAttribute('EPSG');
	    console.info('EPSG='+epsg);
	}

	
	const BBOX_KEY='data-bbox';
	var bbox = [0,0,1,1];

	if (coordTracker.hasAttribute(BBOX_KEY)){
	    bbox = coordTracker.getAttribute(BBOX_KEY).split(',');
	    console.log("data-bbox: ", bbox);
	}
	else {
	    console.log("No attribute [data-bbox], bbox=", bbox);
	}

	
	/// Client rect will be updated (because the page may be scrolled)
	const m = new CoordHandler(bbox, coordTracker.getBoundingClientRect());
	m.setPrecision(coordTracker.getAttribute("data-resolution"));
	
	const selectionRect = panel.querySelector("rect.SELECTOR");

	// Collective, to support cross-actions
	const monitorMove = panel.querySelector('.MONITOR_MOVE');
	const monitorDown = panel.querySelector('.MONITOR_DOWN');
	const monitorUp   = panel.querySelector('.MONITOR_UP');
	// Optional (move also the box)
	const monitorBox  = panel.querySelector('.MONITOR_BOX');

	if (monitorMove){

	    coordTracker.addEventListener("mousemove", (ev) => {
		m.readEvent(ev, m.curr);
		m.update(m.curr, monitorMove);
		// console.log(m.curr)
		if (m.drag && selectionRect){
		    m.updateSpan(selectionRect);
		    if (monitorBox){
			//var transX = 5;
			if (m.curr.x > m.start.x){
			    monitorBox.setAttribute("style", "text-anchor:start");
			}
			else {
			    monitorBox.setAttribute("style", "text-anchor:end");
			}
			
			if (m.curr.y > m.start.y){
			    monitorBox.setAttribute("transform", "translate(0,+20)"); // retrieve STYLE/ font size?
			}
			else {
			    monitorBox.setAttribute("transform", "translate(0,-10)"); // retrieve STYLE/ font size?
			}
		    }
		}
	    })
	    
	}

	
	if (monitorDown){ 
	    coordTracker.addEventListener("mousedown", (ev) => {
		// console.warn(ev)
		m.readEvent(ev, m.start);
		m.update(m.start, monitorDown);
		// Restart drawing, so redraw both corners
		m.curr.x = m.start.x;
		m.curr.y = m.start.y;
		m.update(m.curr, monitorDown);
		m.drag = true;
		if (monitorUp){
		    // kludge
		    monitorUp.textContent = '';
		}
		if (monitorBox){
		    monitorBox.setAttribute("x", m.start.x);
		    monitorBox.setAttribute("y", m.start.y);
		    if (m.start.x > m.curr.x){
		    }
		}
	    })
	}

	
	if (monitorUp){ //  = panel.querySelector('.MONITOR_UP')){

	    coordTracker.addEventListener("mouseup", (ev) => {
		m.readEvent(ev, m.curr);
		m.update(m.curr, monitorUp); // needed?
		m.drag = false;
		console.info([m.start.x, m.start.y, m.curr.x, m.curr.y].join(','));
		console.info("Array size: ", Math.abs(m.curr.x-m.start.x), ',', Math.abs(m.curr.y-m.start.y));
		var bb = m.getGeoBBOX();
		console.info(bb.join(','));
		console.info("Geo size: ", bb[2]-bb[0], ',', bb[3]-bb[1]);
		/*
		navigator.clipboard.writeText(bb).then(
		    () => {
			console.info(bb + ' copied to clipboard')
		    },
		    () => {
			console.warn('clipboard write failed')
		    },
		    );
		*/
	    })
	    
	}


    })
}

function BBox(bbox){
    this.left   = parseFloat(bbox[0]);
    this.top    = parseFloat(bbox[3]);
    this.width  = parseFloat(bbox[2]) - this.left;
    this.height = parseFloat(bbox[1]) - this.top;    
}

function Coord2D(){
    this.x = 0;
    this.y = 0;
}

/*
Coord2D.prototype.readEvent = function(ev){
    this.cbox = ev.target.getBoundingClientRect();
    this.x = ev.clientX - this.cbox.left;
    this.y = ev.clientY - this.cbox.top;
    }
    */

// function CoordHandler(coordTracker, bbox){ // , group?
function CoordHandler(bboxGeo, bboxFrame){ // , group?

    this.bboxFrame = bboxFrame; // coordTracker.getBoundingClientRect();
    this.bboxGeo    = new BBox(bboxGeo);

    this.drag = false;
    this.curr = new Coord2D()
    this.start = new Coord2D()
    // this.up   = new Coord2D()
    // this.group = group;
    // this.resolution = 0;
}

CoordHandler.prototype.readEvent = function(ev, coords){
    this.bboxFrame = ev.target.getBoundingClientRect();
    coords.x = ev.clientX - this.bboxFrame.left; 
    coords.y = ev.clientY - this.bboxFrame.top;
    if (this.precisionFocus){
	coords.x = this.precisionFocus*Math.round(coords.x/this.precisionFocus);
	coords.y = this.precisionFocus*Math.round(coords.y/this.precisionFocus);
    }
}

CoordHandler.prototype.update = function(coord, elem){
    // elem.textContent = this.getPosString(coord.x, coord.y);x    
    elem.textContent = this.getGeoPos(coord.x, coord.y);
};

CoordHandler.prototype.updateSpan = function(elem){
    var width  = Math.abs(this.start.x - this.curr.x);
    var height = Math.abs(this.start.y - this.curr.y);

    if (this.precisionWidth){
	width  = this.precisionWidth*Math.round(width  / this.precisionWidth);	
	height = this.precisionWidth*Math.round(height / this.precisionWidth);	
    }

    if (width && height){
	// toggle visible
	elem.setAttribute("x", Math.min(this.start.x, this.curr.x));
	elem.setAttribute("y", Math.min(this.start.y, this.curr.y));
	elem.setAttribute("width",  width);
	elem.setAttribute("height", height);
	console.info("Size: ", width,',',height);
    }
    else {
	elem.setAttribute("x", 0);
	elem.setAttribute("y", 0);
	elem.setAttribute("width",  0);
	elem.setAttribute("height", 0);
	// console.info('skip update: ', this.start,  ' ', this.curr, ' ', this.up)
	// toggle invisible, or open up
    }
    //elem.textContent = this.getPosString(coord.x, coord.y);
};

CoordHandler.prototype.setPrecision = function(precision){
    if (typeof(precision) === 'number'){
	this.precisionFocus = precision;
	this.precisionWidth = precision;
    }
    else if (typeof(precision) === 'string'){
	precision = precision.split(RegExp('[,:]'))
    }

    console.log(precision)
    
    switch (precision.length) {
    case 2:
	this.precisionFocus = parseInt(precision[0]);
	this.precisionWidth = parseInt(precision[1]);
	break;
    case 1:
	this.precisionFocus = parseInt(precision[0]);
	this.precisionWidth = this.precisionFocus
	break;
    default:
	this.precisionFocus = parseInt(precision[1]);
	this.precisionWidth = precision;
    }
}

CoordHandler.prototype.getRelativeX = function(x){
    //return (x - this.bboxFrame.left)/this.bboxFrame.width;
    return x/this.bboxFrame.width;
}

CoordHandler.prototype.getRelativeY = function(y){
    // return (y - this.bboxFrame.top)/this.bboxFrame.height;
    return y/this.bboxFrame.height;
}

CoordHandler.prototype.getLongitude = function(xRel){
    return this.bboxGeo.left + xRel*this.bboxGeo.width;
}

CoordHandler.prototype.getLatitude = function(yRel){
    return this.bboxGeo.top + (1.0-yRel)*this.bboxGeo.height;
}

/// Get longitude and latitude.
CoordHandler.prototype.getGeoPos = function(x,y){
    return [ Math.round(this.getLongitude(this.getRelativeX(x))),
	     Math.round(this.getLatitude(this.getRelativeY(y))) ]
}

CoordHandler.prototype.getGeoBBOX = function(){
    var x0 = Math.min(this.start.x, this.curr.x)
    var x  = Math.max(this.start.x, this.curr.x)
    var y0 = Math.min(this.start.y, this.curr.y)
    var y  = Math.max(this.start.y, this.curr.y)

    return [].concat(this.getGeoPos(x0, y0 ),
		     this.getGeoPos(x,  y  ))
}

CoordHandler.prototype.getPosString = function(x,y){
    var a = [x, y];
    var xRel = this.getRelativeX(x); 
    var yRel = this.getRelativeY(y); 
    x = Math.round(this.getLongitude(xRel));
    y = Math.round(this.getLatitude(yRel));
    a.push(x);
    a.push(y);
    // a.push(this.drag);
    return a.join(',')
}




/// Default actions. Can be overridden by C++ code XML::JAVASCRIPT_SCOPE (see below)
/**
   Available "context variables":
   this.bboxFrame: SVG element BBOX rect
   this.bboxGeo: BBox object
   TODO: base-update for those above.
   And then store start (upon down) and end (upon up) in image coords 
CoordHandler.prototype.updateMove = CoordHandler.prototype.update
CoordHandler.prototype.updateDown = CoordHandler.prototype.update
CoordHandler.prototype.updateUp   = CoordHandler.prototype.update
 */
)JS";
