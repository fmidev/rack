/// Utilities for coordinate handling 

function Coord2D(){
    this.x = 0;
    this.y = 0;
}

function BBox(bbox){
    if (typeof(bbox) === 'string'){
	bbox = bbox.split(',')
    }
    this.left   = parseFloat(bbox[0]);
    this.top    = parseFloat(bbox[3]);
    this.width  = parseFloat(bbox[2]) - this.left;
    this.height = parseFloat(bbox[1]) - this.top;    
}


BBox.prototype.rx2deg = function(rx){
    return (this.left + rx*this.width).toFixed(1)
}

BBox.prototype.ry2deg = function(ry){
    return (this.top + (1.0-ry)*this.height).toFixed(1)
}

BBox.prototype.rx2metric = function(rx){
    return 1000*Math.round(0.001*(this.left + rx*this.width))
}

BBox.prototype.ry2metric = function(ry){
    return 1000*Math.round(0.001*(this.top + (1.0-ry)*this.height))
}
