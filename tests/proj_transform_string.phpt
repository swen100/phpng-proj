--TEST--
proj_transform_string() function - transform test with strings
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs";
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
$PROJ = proj_create_crs_to_crs($proj_merc, $proj_wgs84);
$coord = "1224514.398726 6621293.7227402";
$p = proj_transform_string($PROJ, $coord);
var_dump($p);

proj_transform_string();
proj_transform_string($PROJ);
proj_transform_string([], $coord);
proj_transform_string($PROJ, []);

?>
--EXPECTF--
array(1) {
  [0]=>
  array(3) {
    ["x"]=>
    float(11)
    ["y"]=>
    float(51)
    ["z"]=>
    float(0)
  }
}

Warning: proj_transform_string() expects exactly 2 parameters, 0 given in %s

Warning: proj_transform_string() expects exactly 2 parameters, 1 given in %s

Warning: proj_transform_string() expects parameter 1 to be resource, array given in %s

Warning: proj_transform_string() expects parameter 2 to be string, array given in %s