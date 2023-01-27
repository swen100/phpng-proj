--TEST--
proj4_transform_point() function - transform test with single points
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

// strings
$x = "1224520";
$y = "6621300";
$p = proj4_transform_point($proj_merc, $proj_wgs84, $x, $y);
var_dump($p);

// float
$x = 1224520.0;
$y = 6621300.0;
$p = proj4_transform_point($proj_merc, $proj_wgs84, $x, $y);
var_dump($p);

try {
    proj4_transform_point($proj_merc);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_point($proj_merc, $proj_wgs84);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_point($proj_merc, $proj_wgs84, $x);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_point(null, $proj_wgs84, $x, $y);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_point($proj_merc, null, $x, $y);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_point($proj_merc, $proj_wgs84, null, $y);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}



?>
--EXPECTF--
array(3) {
  ["x"]=>
  float(11.%d)
  ["y"]=>
  float(51.%d)
  ["z"]=>
  float(0)
}
array(3) {
  ["x"]=>
  float(11.%d)
  ["y"]=>
  float(51.%d)
  ["z"]=>
  float(0)
}
proj4_transform_point() expects at least 4 arguments, 1 given
proj4_transform_point() expects at least 4 arguments, 2 given
proj4_transform_point() expects at least 4 arguments, 3 given
proj4_transform_point(): Argument #1 ($src) must be of type resource, null given
proj4_transform_point(): Argument #2 ($tgt) must be of type resource, null given

Deprecated: proj4_transform_point(): Passing null to parameter #3 ($x) of type float is deprecated in %s