/** Automatically generated from " src/drain/js/coords.js " */
namespace javascript {
const char *coords = R"JS(
/// Utilities for coordinate handling 

function BBox(bbox){
    if (typeof(bbox) === 'string'){
	bbox = bbox.split(',')
    }
    this.left   = parseFloat(bbox[0]);
    this.top    = parseFloat(bbox[3]);
    this.width  = parseFloat(bbox[2]) - this.left;
    this.height = parseFloat(bbox[1]) - this.top;    
}

function Coord2D(){
    this.x = 0;
    this.y = 0;
}
)JS";
} // javascript::

