--TEST--
proj_transform_array() function - transform test with arrays
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
$PROJ = proj_create_crs_to_crs($proj_wgs84, $proj_merc);

$x = "11";
$y = "51";
$coords = [[11, 51], [$x, $y], [11.5, 51.5, 20], "11 51", "11.5 51.5 20"];
$transformed = proj_transform_array($PROJ, $coords);
var_dump($transformed);

try {
    proj_transform_array();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_array($PROJ);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_array(null, $coords);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_array($PROJ, null);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_array($PROJ, "test");
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

/*
try {
    $coords = [[NULL, 12]];
    $transformed = proj_transform_array($PROJ, $coords);
var_dump($transformed);
} catch (\Error $e) {
    echo $e->getMessage();
}

try {
    $coords = [[new stdClass, 12]];
    $transformed = proj_transform_array($PROJ, $coords);
var_dump($transformed);
} catch (\Error $e) {
    echo $e->getMessage();
}

try {
    $coords = [new stdClass];
    $transformed = proj_transform_array($PROJ, $coords);
var_dump($transformed);
} catch (TypeError $e) {
    echo $e->getMessage();
}*/

?>
--EXPECTF--
array(5) {
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
    float(1224514.%d)
    ["y"]=>
    float(6621293.%d)
    ["z"]=>
    float(0)
  }
  [2]=>
  array(3) {
    ["x"]=>
    float(1280174.%d)
    ["y"]=>
    float(6710219.%d)
    ["z"]=>
    float(20)
  }
  [3]=>
  array(3) {
    ["x"]=>
    float(1224514.%d)
    ["y"]=>
    float(6621293.%d)
    ["z"]=>
    float(0)
  }
  [4]=>
  array(3) {
    ["x"]=>
    float(1280174.%d)
    ["y"]=>
    float(6710219.%d)
    ["z"]=>
    float(20)
  }
}
proj_transform_array() expects exactly 2 arguments, 0 given
proj_transform_array() expects exactly 2 arguments, 1 given
proj_transform_array(): Argument #1 ($Proj) must be of type resource, null given
proj_transform_array(): Argument #2 ($points) must be of type array, null given
proj_transform_array(): Argument #2 ($points) must be of type array, string given