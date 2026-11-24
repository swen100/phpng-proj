--TEST--
proj_is_geocent() function - basic test for proj_is_geocent()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$pj = proj_create('+proj=geocent +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
var_dump(proj_is_geocent($pj));

$pj = proj_create('+proj=latlong +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56 +units=m');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
var_dump(proj_is_geocent($pj));

// projected CRS (UTM) is not geocentric
var_dump(proj_is_geocent(proj_create('EPSG:25832')));

// geocentric CRS and a transformation into one
var_dump(proj_is_geocent(proj_create('EPSG:4978')));
var_dump(proj_is_geocent(proj_create_crs_to_crs('EPSG:4326', 'EPSG:4978')));
var_dump(proj_is_geocent(proj_create_crs_to_crs('EPSG:4326', 'EPSG:25832')));
?>
--EXPECT--
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
bool(false)