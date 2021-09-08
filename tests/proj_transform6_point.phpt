--TEST--
proj_transform() function - basic test for proj_transform_point()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs";
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";

$pj = proj_create_crs_to_crs($proj_merc, $proj_wgs84);
var_dump($pj);

// strings
$x = "1224514.398726";
$y = "6621293.7227402";
$p = proj6_transform_point($pj, $x, $y);
var_dump($p);

// float
$x = 1224514.398726;
$y = 6621293.7227402;
$p = proj6_transform_point($pj, $x, $y);
var_dump($p);

try {
    proj6_transform_point();
    proj6_transform_point($pj);
    proj6_transform_point($pj, $x);
    proj6_transform_point(null, $x, $y);
    proj6_transform_point($pj, null, $y);
    proj6_transform_point($pj, $x, []);
} catch (Exception $e) {
    echo $e->getMessage();
}
?>
--EXPECTF--
resource(%d) of type (Proj)
array(3) {
  ["x"]=>
  float(11)
  ["y"]=>
  float(51)
  ["z"]=>
  float(6.9%s)
}
array(3) {
  ["x"]=>
  float(11)
  ["y"]=>
  float(51)
  ["z"]=>
  float(6.9%s)
}

Warning: proj6_transform_point() expects at least 3 parameters, 0 given in %s

Warning: proj6_transform_point() expects at least 3 parameters, 1 given in %s

Warning: proj6_transform_point() expects at least 3 parameters, 2 given in %s

Warning: proj6_transform_point() expects parameter 1 to be resource, null given in %s

Warning: proj6_transform_point() expects parameter 3 to be float, array given in %s