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
$coord = "1224520 6621300";
$p = proj_transform_string($PROJ, $coord);
var_dump($p);

try {
    proj_transform_string();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_string($PROJ);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_string([], $coord);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_string($PROJ, []);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

?>
--EXPECTF--
array(1) {
  [0]=>
  array(3) {
    ["x"]=>
    float(11.%d)
    ["y"]=>
    float(51.%d)
    ["z"]=>
    float(0)
  }
}
proj_transform_string() expects exactly 2 arguments, 0 given
proj_transform_string() expects exactly 2 arguments, 1 given
proj_transform_string(): Argument #1 ($Proj) must be of type resource, array given
proj_transform_string(): Argument #2 ($geometry) must be of type string, array given