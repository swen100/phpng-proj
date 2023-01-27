--TEST--
proj_distance() functions - get distance tests with single points
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$proj_merc = proj_create('EPSG:3857');
$proj_wgs84 = proj_create('EPSG:4326');

# Brocken
$x1 = 1181721;
$y1 = 6763888;
$z1 = 1140;
$x2 = 1195112;
$y2 = 6754023;
$z2 = 456;

$dist = proj_distance_xy($x1, $y1, $x2, $y2);
var_dump($dist);

$dist = proj_distance_xyz($x1, $y1, $z1, $x2, $y2, $z2);
var_dump($dist);

$x1 = deg2rad(10.615561);
$y1 = deg2rad(51.799148);
$z1 = 1140;
$x2 = deg2rad(10.735874);
$y2 = deg2rad(51.744303);
$z2 = 456;

# not working
#$dist = proj_distance_lp($proj_wgs84, $x1, $y1, $x2, $y2);
#var_dump($dist);

#$dist = proj_distance_lpz($proj_wgs84, $x1, $y1, $z1, $x2, $y2, $z2);
#var_dump($dist);


try {
    proj_distance_xy();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_xyz();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_xy($x1, $y1, $x2);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_xyz($x1, $y1, $z1, $x2, $y2);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_xy($x1, $y1, $x2, null);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_xyz($x1, $y1, $z1, $x2, $y2, null);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_lp(null, $x1, $y1, $x2, $y2);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_distance_lpz(null, $x1, $y1, $z1, $x2, $y2, $z2);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

?>
--EXPECTF--
float(16632.%f)
float(16646.%f)
proj_distance_xy() expects exactly 4 arguments, 0 given
proj_distance_xyz() expects exactly 6 arguments, 0 given
proj_distance_xy() expects exactly 4 arguments, 3 given
proj_distance_xyz() expects exactly 6 arguments, 5 given

Deprecated: proj_distance_xy(): Passing null to parameter #4 ($y2) of type float is deprecated in %s

Deprecated: proj_distance_xyz(): Passing null to parameter #6 ($z2) of type float is deprecated in %s
proj_distance_lp(): Argument #1 ($Proj) must be of type resource, null given
proj_distance_lpz(): Argument #1 ($Proj) must be of type resource, null given