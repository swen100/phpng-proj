--TEST--
proj_transform() function - basic test for proj_transform_point()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
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
$x = "1224514.398726";
$y = "6621293.7227402";
$p = proj_transform_point($proj_merc, $proj_wgs84, $x, $y);
var_dump($p);

// float
$x = 1224514.398726;
$y = 6621293.7227402;
$p = proj_transform_point($proj_merc, $proj_wgs84, $x, $y);
var_dump($p);

try {
    proj_transform_point($proj_merc);
    proj_transform_point($proj_merc, $proj_wgs84);
    proj_transform_point($proj_merc, $proj_wgs84, $x);
    proj_transform_point(null, $proj_wgs84, $x, $y);
    proj_transform_point($proj_merc, null, $x, $y);
    proj_transform_point($proj_merc, $proj_wgs84, null, $y);
    proj_transform_point($proj_merc, $proj_wgs84, $x, []);
} catch (Exception $e) {
    echo $e->getMessage();
}
?>
--EXPECTF--
array(3) {
  ["x"]=>
  float(11)
  ["y"]=>
  float(51)
  ["z"]=>
  float(0)
}
array(3) {
  ["x"]=>
  float(11)
  ["y"]=>
  float(51)
  ["z"]=>
  float(0)
}

Warning: proj_transform_point() expects at least 4 parameters, 1 given in %s

Warning: proj_transform_point() expects at least 4 parameters, 2 given in %s

Warning: proj_transform_point() expects at least 4 parameters, 3 given in %s

Warning: proj_transform_point() expects parameter 1 to be resource, null given in %s

Warning: proj_transform_point() expects parameter 2 to be resource, null given in %s

Warning: proj_transform_point() expects parameter 4 to be float, array given in %s