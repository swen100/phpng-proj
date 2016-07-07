phpng-proj
=========

A php extension for proj.4

Code example (see: [ProjAPI](http://trac.osgeo.org/proj/wiki/ProjAPI)):
-------------
	<?php  
	$pj_merc = pj_init_plus("+proj=merc +ellps=clrk66 +lat_ts=33");  
	$pj_latlong = pj_init_plus("+proj=latlong +ellps=clrk66");  
	if ($pj_merc !== false && $pj_latlong !== false) {  
		$x = deg2rad(-16);  
		$y = deg2rad(20.25);  
		$transformed = pj_transform($pj_merc, $pj_latlong, 1, 0, $x, $y);  
		print 'latitude: '.$transformed['x'].'<br />';  
		print 'longitude: '.$transformed['y'].'<br />';  
	}  
	?>

Output:
-------
	latitude: -1495284.2114735  
	longitude: 1920596.7899174

Code example 2:
-------------
	<?php  
	$pj_merc = pj_init_plus("+proj=merc +ellps=clrk66 +lat_ts=33");  
	$pj_latlong = pj_init_plus("+proj=latlong +ellps=clrk66");  
	if ($pj_merc !== false && $pj_latlong !== false) {  
		$x = array(deg2rad(-16), deg2rad(-25));  
		$y = array(deg2rad(20.25), deg2rad(13.2));  
		$transformed = pj_transform($pj_merc, $pj_latlong, count($x), 0, $x, $y);  
		print_r($transformed);  
	}  
	?>

Output:
-------
	Array
	(
		[x] => Array
			(
				[0] => -1495284.2114735
				[1] => -2336381.5804273
			)

		[y] => Array
			(
				[0] => 1920596.7899174
				[1] => 1236391.9797675
			)

		[z] => Array
			(
				[0] => 0
				[1] => 0
			)

	)

API Functions
=============
Basic API
---------
**resource pj_init_plus(string definition);**  
Create a Proj.4 resource coordinate system object from the string definition.  
  
**int pj_transform(resource srcdefn, resource dstdefn, int point_count, int point_offset, mixed x, mixed y, mixed z);**  
Transform the x/y/z points from the source coordinate system to the destination coordinate system.   
x, y and z can be double, int or a numeric string.  
x, y and z can also be an array of double, int or numeric string values.  
  
**void pj_free(resource pj);**  
Frees all resources associated with pj.  

Advanced Functions
------------------
**boolean pj_is_latlong(resource pj);**  
Returns true if the passed coordinate system is geographic (proj=latlong).  
  
**boolean pj_is_geocent(resource pj);**  
Returns true if the coordinate system is geocentric (proj=geocent).  
  
**string pj_get_def(resource pj, int options);**  
Returns the PROJ.4 initialization string suitable for use with pj_init_plus() that would produce this coordinate system, but with the definition expanded as much as possible (for instance +init= and +datum= definitions).  
  
**resource pj_latlong_from_proj(resource pj_in);**  
Returns a new coordinate system definition which is the geographic coordinate (lat/long) system underlying pj_in.  

Environment Functions
---------------------
**void pj_deallocate_grids();**  
Frees all resources associated with loaded and cached datum shift grids.  
  
**string pj_strerrno(int);**  
Returns the error text associated with the passed in error code.  
  
**int pj_get_errno_ref();**  
Returns an integer value that can be used for the pj_strerrno(int) function.  
  
**string pj_get_release();**  
Returns an internal string describing the release version. 
