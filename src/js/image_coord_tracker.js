/// Utility

/// Realign text
function flipFlop(textElem, horz, vert){
    
    if (horz){
	// textElem.setAttribute('text-anchor', 'end');
	textElem.style.setProperty('text-anchor', 'end');
    }
    else {
	// textElem.setAttribute('text-anchor', 'start');
	textElem.style.setProperty('text-anchor', 'start');
    }

    if (vert){
	const offset = -10; 
	textElem.setAttribute('transform', 'translate(0,'+offset+')'); 
    }
    else {
	const offset = 20; 
	textElem.setAttribute('transform', 'translate(0,'+offset+')'); 
    }
}

/// Add mouse listerners 
function image_coord_tracker(){

    const groups = document.querySelectorAll('.MOUSE');

    groups.forEach(group => {

	// Plane listening to mouse events
	const coordTracker = group.querySelector('.MOUSE_LISTENER');
	if (!coordTracker){
	    console.info(group);
	    console.error('elem .MOUSE found without child .MOUSE_LISTENER');
	    return;
	}

	// var epsg = null;
	if (group.hasAttribute('data-epsg')){
	    var epsg = group.getAttribute('data-epsg');
	    console.info('EPSG='+epsg);
	}
	
	// const BBOX_KEY='data-bbox';
	var bbox = [0,0,1,1];
	if (group.hasAttribute('data-bbox')){
	    bbox = group.getAttribute('data-bbox').split(',');
	    console.log('data-bbox: ', bbox);
	}
	else {
	    console.log('No attribute [data-bbox], bbox=', bbox);
	}

	
	/// Client rect will be updated (because the page may be scrolled)
	const m = new CoordHandler(bbox, coordTracker.getBoundingClientRect());
	m.setPrecision(coordTracker.getAttribute('data-resolution'));

	// Visualisation
	const selectionRect = group.querySelector('rect.SELECTOR');
	// Collective, to support cross-actions
	const monitorMain = group.querySelector('.MONITOR'); // top level group for others
	const monitorMove = group.querySelector('.MONITOR_MOVE');
	const monitorDown = group.querySelector('.MONITOR_DOWN');
	const monitorUp   = group.querySelector('.MONITOR_UP');
	// Optional (move also the box)
	const monitorDrag  = group.querySelector('.MONITOR_DRAG');

	if (monitorMove){
	    
	    coordTracker.addEventListener('mousemove', (ev) => {
		m.readEvent(ev, m.curr);
		m.update(m.curr, monitorMove);
		// console.log(m.curr)
		// Todo: add free/iddle coord monitor: option to monitor of coord (if no-dragging)
		if (m.drag && selectionRect){
		    m.updateSpan(selectionRect);
		    if (monitorDrag){
			// Toggle text direction away from rect/image borders 
			flipFlop(monitorDrag, m.curr.x < m.start.x, m.curr.y < m.start.y);
		    }
		}
	    })
	    
	}

	
	if (monitorDown){ 
	    coordTracker.addEventListener('mousedown', (ev) => {
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
		if (monitorDrag){
		    monitorDrag.setAttribute('x', m.start.x);
		    monitorDrag.setAttribute('y', m.start.y);
		    // if (m.start.x > m.curr.x){}
		}
		
		if (monitorMain){
		    monitorMain.style.visibility = 'visible';
		}

	    })
	}

	
	if (monitorUp){ 

	    coordTracker.addEventListener('mouseup', (ev) => {
		m.readEvent(ev, m.curr);
		m.update(m.curr, monitorUp); // needed?
		m.drag = false;
		if ((m.curr.x != m.start.x) && (m.curr.y != m.start.y)){
		    console.info([m.start.x, m.start.y, m.curr.x, m.curr.y].join(','));
		    console.info('Array size: ', Math.abs(m.curr.x-m.start.x), ',', Math.abs(m.curr.y-m.start.y));
		    var bb = m.getGeoBBOX();
		    console.info(bb.join(','));
		    console.info('Geo size: ', bb[2]-bb[0], ',', bb[3]-bb[1]);		    
		}
		else {

		    if (monitorMain){
			monitorMain.style.visibility = 'hidden';
		    }
		    
		    if (monitorDown){
			monitorDown.textContent = '*';
		    }
		    if (monitorUp){
			monitorUp.textContent = '*';
		    }		    
		}
	    })
	    
	}


    })
}


/// Redefine
CoordHandler.prototype.update = function(coord, elem){
    // elem.textContent = this.getPosString(coord.x, coord.y);x    
    elem.textContent = this.getGeoPos(coord.x, coord.y);
};



/// Redefinition
CoordHandler.prototype.updateSpanElem = function(elem, x,y,width,height){
    if (width && height){
	// toggle visible
	elem.setAttribute('x', Math.min(this.start.x, this.curr.x));
	elem.setAttribute('y', Math.min(this.start.y, this.curr.y));
	elem.setAttribute('width',  width);
	elem.setAttribute('height', height);
	if (this.precisionWidth){ // Else no need
	    console.info('Size: ', width,',',height);
	} 
    }
    else {
	elem.setAttribute('x', 0);
	elem.setAttribute('y', 0);
	elem.setAttribute('width',  0);
	elem.setAttribute('height', 0);
	// console.info('skip update: ', this.start,  ' ', this.curr, ' ', this.up)
	// toggle invisible, or open up
    }
}


