/** Automatically wrapped from " src/js/set_image_coord_tracker.js " */

const char* set_image_coord_tracker = R"JS(<![CDATA[
function set_image_coord_tracker(img, func){
    const bbox  = img.getBoundingClientRect();
    img.addEventListener("mousemove", (ev) => {
	func((ev.clientX - bbox.left)/bbox.width, (ev.clientY - bbox.top)/bbox.height)
    })
};
//]]>)JS";
