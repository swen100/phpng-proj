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
$proj_merc = proj_create("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($proj_merc === false) {
	die(proj_get_errno_string(proj_get_errno($proj_merc)));
}
$proj_wgs84 = proj_create("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
	die(proj_get_errno_string(proj_get_errno($proj_wgs84)));
}
$coord = "1224514.398726 6621293.7227402";
$p = proj_transform_string($proj_merc, $proj_wgs84, $coord);
var_dump($p);

proj_transform_string($proj_merc);
proj_transform_string($proj_merc, $proj_wgs84);
proj_transform_string([], $proj_wgs84, $coord);
proj_transform_string($proj_merc, [], $coord);
proj_transform_string($proj_merc, $proj_wgs84, []);

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

Warning: proj_transform_string() expects exactly 3 parameters, 1 given in %s

Warning: proj_transform_string() expects exactly 3 parameters, 2 given in %s

Warning: proj_transform_string() expects parameter 1 to be resource, array given in %s

Warning: proj_transform_string() expects parameter 2 to be resource, array given in %s

Warning: proj_transform_string() expects parameter 3 to be string, array given in %s