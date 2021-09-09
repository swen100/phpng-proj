--TEST--
proj_transform() function - basic array test with arrays for proj_transform_array()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) {
    echo 'skip proj.4 extension not available';
}
?>
--FILE--
<?php
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs";
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
$PROJ = proj_create_crs_to_crs($proj_wgs84, $proj_merc);

$coords = [[11, 51], [11.5, 51.5, 20], "11 51", "11.5 51.5 20"];
$transformed = proj6_transform_array($PROJ, $coords);
var_dump($transformed);

proj6_transform_array();
proj6_transform_array($PROJ);
proj6_transform_array(null, $coords);
proj6_transform_array($PROJ, null);
proj6_transform_array($PROJ, "test");

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

Warning: proj6_transform_array() expects exactly 2 parameters, 0 given in %s

Warning: proj6_transform_array() expects exactly 2 parameters, 1 given in %s

Warning: proj6_transform_array() expects parameter 1 to be resource, null given in %s

Warning: proj6_transform_array() expects parameter 2 to be array, null given in %s

Warning: proj6_transform_array() expects parameter 2 to be array, string given in %s