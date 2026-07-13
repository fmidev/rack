/// Utilities for coordinate handling 

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


function CoordHandler(bboxGeo, bboxFrame){
    this.bboxFrame = bboxFrame; // coordTracker.getBoundingClientRect();
    this.bboxGeo   = new BBox(bboxGeo);
    this.drag  = false;
    this.curr  = new Coord2D()
    this.start = new Coord2D()
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

CoordHandler.prototype.update = function(coord, elem){};
CoordHandler.prototype.updateSpanElem = function(elem, x,y,width,height){};

CoordHandler.prototype.updateSpan = function(elem){

    var width  = Math.abs(this.start.x - this.curr.x);
    var height = Math.abs(this.start.y - this.curr.y);

    if (this.precisionWidth){
	width  = this.precisionWidth*Math.round(width  / this.precisionWidth);	
	height = this.precisionWidth*Math.round(height / this.precisionWidth);	
    }

    this.updateSpanElem(elem,
			Math.min(this.start.x, this.curr.x),
			Math.min(this.start.y, this.curr.y),
			width,
			height);
			    
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

    console.log("precision: ", precision, "px")
    
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
