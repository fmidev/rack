/** Automatically wrapped from " src/js/image_value_tracker.js " */

const char* image_value_tracker = R"JS(<![CDATA[
function set_image_value_tracker(img, func){


    // var img = document.getElementById('rgb');
    const w = img.naturalWidth  | 0;
    const h = img.naturalHeight | 0;
    const canvas = new OffscreenCanvas(w, h);
    const ctx = canvas.getContext("2d", { willReadFrequently: true });
    // Draw and read
    ctx.clearRect(0, 0, w, h);
    ctx.drawImage(img, 0, 0);
    const imageData = ctx.getImageData(0, 0, w, h);
    const data = imageData.data; // Uint8ClampedArray, RGBA

    // Grayscale value at (x,y): for grayscale PNG, R==G==B
    function grayAt(x, y) {
	if (x < 0 || y < 0 || x >= w || y >= h) return null;
	const i = (y * w + x) * 4;
	return data[i]; // R
    }
    console.info('adding image value listener to ', img)
    
    const bbox  = img.getBoundingClientRect();
    img.addEventListener("mousemove", (ev) => {
	const x = ev.clientX - bbox.left;
	const y = ev.clientY - bbox.top;
	console.info(grayAt(x, y))
	// const i = x / bbox.width;
	// const j = y / bbox.height;
	//func(i,j)
    })
};
//]]>)JS";
