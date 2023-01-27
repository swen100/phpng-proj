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
$proj_merc = proj_create("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($proj_merc === false) {
    die(proj_get_errno_string(proj_get_errno($proj_merc)));
}
$proj_wgs84 = proj_create("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
    die(proj_get_errno_string(proj_get_errno($proj_wgs84)));
}
$coords = [[11, 51], [11.5, 51.5, 20], "11 51", "11.5 51.5 20"];
$transformed = proj4_transform_array($proj_wgs84, $proj_merc, $coords);
var_dump($transformed);

try {
    proj4_transform_array();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_array($proj_wgs84);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_array($proj_wgs84, $proj_merc);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_array(null, $proj_merc, $coords);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_array($proj_wgs84, null, $coords);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj4_transform_array($proj_wgs84, $proj_merc, "test");
} catch (TypeError $exc) {
    echo $exc->getMessage();
}

?>
--EXPECTF--
array(4) {
  [0]=>
  array(3) {
    ["x"]=>
    float(1224514.%d)
    ["y"]=>
    float(6621293.%d)
    ["z"]=>
    float(0)
  }
  [1]=>
  array(3) {
    ["x"]=>
    float(1280174.%d)
    ["y"]=>
    float(6710219.%d)
    ["z"]=>
    float(20)
  }
  [2]=>
  array(3) {
    ["x"]=>
    float(1224514.%d)
    ["y"]=>
    float(6621293.%d)
    ["z"]=>
    float(0)
  }
  [3]=>
  array(3) {
    ["x"]=>
    float(1280174.%d)
    ["y"]=>
    float(6710219.%d)
    ["z"]=>
    float(20)
  }
}
proj4_transform_array() expects exactly 3 arguments, 0 given
proj4_transform_array() expects exactly 3 arguments, 1 given
proj4_transform_array() expects exactly 3 arguments, 2 given
proj4_transform_array(): Argument #1 ($srcProj) must be of type resource, null given
proj4_transform_array(): Argument #2 ($tgtProj) must be of type resource, null given
proj4_transform_array(): Argument #3 ($points) must be of type array, string given