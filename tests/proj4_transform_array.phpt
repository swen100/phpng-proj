--TEST--
proj4_transform_array() function - transform test with arrays
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$proj_merc = proj4_create("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($proj_merc === false) {
    die(proj_get_errno_string(proj_get_errno($proj_merc)));
}
$proj_wgs84 = proj4_create("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
    die(proj_get_errno_string(proj_get_errno($proj_wgs84)));
}
$coords = [[11, 51], [11.5, 51.5, 20], "11 51", "11.5 51.5 20"];
$transformed = proj4_transform_array($proj_wgs84, $proj_merc, $coords);
var_dump($transformed);

proj4_transform_array();
proj4_transform_array($proj_wgs84);
proj4_transform_array($proj_wgs84, $proj_merc);
proj4_transform_array(null, $proj_merc, $coords);
proj4_transform_array($proj_wgs84, null, $coords);
proj4_transform_array($proj_wgs84, $proj_merc, "test");

?>
--EXPECTF--
array(4) {
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
  [2]=>
  array(3) {
    ["x"]=>
    float(1224514.398726)
    ["y"]=>
    float(6621293.7227402)
    ["z"]=>
    float(0)
  }
  [3]=>
  array(3) {
    ["x"]=>
    float(1280174.1441226)
    ["y"]=>
    float(6710219.0832207)
    ["z"]=>
    float(20)
  }
}

Warning: proj4_transform_array() expects exactly 3 parameters, 0 given in %s

Warning: proj4_transform_array() expects exactly 3 parameters, 1 given in %s

Warning: proj4_transform_array() expects exactly 3 parameters, 2 given in %s

Warning: proj4_transform_array() expects parameter 1 to be resource, null given in %s

Warning: proj4_transform_array() expects parameter 2 to be resource, null given in %s

Warning: proj4_transform_array() expects parameter 3 to be array, string given in %s