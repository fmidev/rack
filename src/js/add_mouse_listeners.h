/** Automatically wrapped from " src/js/add_mouse_listeners.js " */

const char* add_mouse_listeners = R"JS(<![CDATA[
function add_coord_monitor(frame, monitor){

    
    
    const BBOX_KEY='data-bbox';
    if (frame.hasAttribute(BBOX_KEY)){
	const bbox = frame.getAttribute(BBOX_KEY).split(',');
	const xLL = parseFloat(bbox[0]);
	const yLL = parseFloat(bbox[1]);
	const xUR = parseFloat(bbox[2]);
	const yUR = parseFloat(bbox[3]);	    
	console.log(bbox);
	set_image_coord_tracker(frame, (x,y) => {
	    const width  = xUR-xLL;
	    const height = yUR-yLL;
	    monitor.textContent = ''+Math.round(xLL + x*width)+','+Math.round(yLL + (1.0-y)*height)
	})
    }
    else {
	console.warn('element having class[', clsName, '] is missing attribute "' + BBOX_VARNAME, '"')
	console.warn(frame)
    }

}

function add_data_monitor(frame, monitor){
    var data_arrays = panel.getElementsByClassName('DATA_ARRAY')
    if (data_arrays.length>0){
	var data_array = data_arrays[0]
	const encoding = data_array.getAttribute('data-encoding').split(',');
	// Under constr...
 	set_image_coord_tracker(frame, (x,y) => {
	    const width  = xUR-xLL;
	    const height = yUR-yLL;
	    console.info(''+x+','+y)
	})
    }
    
}

function add_coord_tracker(){


    //var elems = document.getElementsByClassName('MOUSE');
    const elems = document.querySelectorAll(".MOUSE");

    elems.forEach(panel => {

	const frame   = panel.querySelector(".IMAGE_BORDER");
	const monitor = panel.querySelector('.MONITOR')
	
	// Coordinate monitor
	add_coord_monitor(frame, monitor)

	var metadata  = panel.querySelector(".SHARED");
	var epsg = metadata.getAttribute('EPSGx');
	if (metadata.hasAttribute('EPSG')){
	    epsg = metadata.getAttribute('EPSG');
	    console.info('EPSG='+epsg)
	}

    })
}
//]]>)JS";
