--TEST--
proj_get_def() function - basic test for proj_get_def()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
// with EPSG-code
$pj = proj_create_crs_to_crs("EPSG:3857", "EPSG:25832");
var_dump(proj_get_def($pj));

$pj = proj_create("EPSG:3857");
var_dump(proj_get_def($pj));

$pj = proj_create("EPSG:4326");
var_dump(proj_get_def($pj));

// with proj-string
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +units=m +k=1.0 +nadgrids=@null +no_defs";
$pj = proj_create_crs_to_crs($proj_wgs84, $proj_merc);
var_dump(proj_get_def($pj));

?>
--EXPECT--
string(109) "proj=pipeline step inv proj=webmerc lat_0=0 lon_0=0 x_0=0 y_0=0 ellps=WGS84 step proj=utm zone=32 ellps=GRS80"
string(0) ""
string(0) ""
string(114) "proj=pipeline step proj=unitconvert xy_in=deg xy_out=rad step proj=webmerc lat_0=0 lon_0=0 x_0=0 y_0=0 ellps=WGS84"