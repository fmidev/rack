/** Automatically generated from " src/drain/js/textbox_flipper.js " */

#ifndef JS_INCLUDE_textbox_flipper
#define JS_INCLUDE_textbox_flipper


namespace javascript {
const char* textbox_flipper = R"JS(// Source:  src/drain/js/textbox_flipper.js

/// Flips text inside a group - usually to keep it visible (inside image area).
/**
   
*/
function flipTextBox(elem, horzFlip, vertFlip){

    if (!elem){
	return
    };

    // var adapterElemgroup.querySelector('.');
    
    var textAnchor = 'middle';
    
    if (!horzFlip)
	horzFlip=0;
    
    if (horzFlip > 0){
	textAnchor='end';}
    else if (horzFlip < 0){
	textAnchor='start';}
    
    if (!vertFlip)
	vertFlip=0;
    
    var dy = elem.getAttribute('data-height')
    if (!dy)
	dy = 0;
    
    if (vertFlip > 0){
        dy = -dy;}
    else if (vertFlip < 0){
        dy = 0;}
    else {
        dy = -dy/2;}

    elem.style.textAnchor = textAnchor;
    elem.setAttribute('transform', 'translate(0,'+dy+')');
    
}

/// Flips text inside a group - usually to keep it visible (inside image area).
/**

   If threshold is over 0.5, the flipping becomes binary: left-right and top-bottom.
   In addition, there is inertia.
   
   \param rx - relative X coord (0...1)
   \param ry - relative Y coord (0...1)
   \param threshold - threshold (0...1)
   
*/
function flipTextBoxWithThreshold(elem, rx, ry, threshold){

    var horzFlip = getThresholdedFlip('horzFlip', rx, threshold);
    var vertFlip = getThresholdedFlip('vertFlip', ry, threshold);
    flipTextBox(elem, horzFlip, vertFlip);
    // console.info(`FLIP ${rx},${ry}  ${horzFlip},${vertFlip}`)
   
}

function getThresholdedFlip(name, value, threshold){

    var flip = 0;
    if (threshold <= 0.5){
	if (value < threshold){
	    flip = -1;
	}
	else if (value > (1.0-threshold)){
	    flip = +1;
	}
    }
    else {
	// Lazy flip (inertia)
	if (name in elem)
	    flip = elem[name];
	
	if (rx < (1.0-threshold)){
	    flip = -1;
	}
	else if (rx > threshold){
	    flip = +1;
	}
	
	elem[name] = flip;
    }

    return flip;
    
}
)JS";
} // javascript::
#endif
