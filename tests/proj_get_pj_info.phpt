--TEST--
proj_get_pj_info() function - basic test for proj_get_pj_info()
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
var_dump(proj_get_pj_info($pj));

$pj = proj_create("EPSG:3857");
var_dump(proj_get_pj_info($pj));

$pj = proj_create("EPSG:4326");
var_dump(proj_get_pj_info($pj));

// with proj-string
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +units=m +k=1.0 +nadgrids=@null +no_defs";
$pj = proj_create_crs_to_crs($proj_wgs84, $proj_merc);
var_dump(proj_get_pj_info($pj));

?>
--EXPECT--
array(5) {
  ["id"]=>
  string(8) "pipeline"
  ["definition"]=>
  string(109) "proj=pipeline step inv proj=webmerc lat_0=0 lon_0=0 x_0=0 y_0=0 ellps=WGS84 step proj=utm zone=32 ellps=GRS80"
  ["description"]=>
  string(97) "Inverse of Popular Visualisation Pseudo-Mercator + Inverse of ETRS89 to WGS 84 (1) + UTM zone 32N"
  ["accuracy"]=>
  float(1)
  ["has_inverse"]=>
  int(1)
}
array(5) {
  ["id"]=>
  string(0) ""
  ["definition"]=>
  string(0) ""
  ["description"]=>
  string(24) "WGS 84 / Pseudo-Mercator"
  ["accuracy"]=>
  float(-1)
  ["has_inverse"]=>
  int(0)
}
array(5) {
  ["id"]=>
  string(0) ""
  ["definition"]=>
  string(0) ""
  ["description"]=>
  string(6) "WGS 84"
  ["accuracy"]=>
  float(-1)
  ["has_inverse"]=>
  int(0)
}
array(5) {
  ["id"]=>
  string(8) "pipeline"
  ["definition"]=>
  string(114) "proj=pipeline step proj=unitconvert xy_in=deg xy_out=rad step proj=webmerc lat_0=0 lon_0=0 x_0=0 y_0=0 ellps=WGS84"
  ["description"]=>
  string(62) "axis order change (2D) + Popular Visualisation Pseudo-Mercator"
  ["accuracy"]=>
  float(0)
  ["has_inverse"]=>
  int(1)
}