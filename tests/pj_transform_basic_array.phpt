--TEST--
pj_transform() function - basic array test for pj_transform()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$proj_merc = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($proj_merc === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$proj_wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$coords = ["11 51", "11.5 51.5 20"];
$transformed = pj_transform_array( $proj_wgs84, $proj_merc, $coords );
var_dump($transformed);
?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["x"]=>
    float(1224514.398726)
    ["y"]=>
    float(6621293.7227402)
    ["z"]=>
    float(0)
  }
  [1]=>
  array(3) {
    ["x"]=>
    float(1280174.1441226)
    ["y"]=>
    float(6710219.0832207)
    ["z"]=>
    float(20)
  }
}