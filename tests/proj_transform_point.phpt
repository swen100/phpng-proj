--TEST--
proj_transform_point() function - transform test with single points
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

$pj = proj_create_crs_to_crs($proj_merc, $proj_wgs84);
var_dump($pj);

// strings
$x = "1224520";
$y = "6621300";
$p = proj_transform_point($pj, $x, $y);
var_dump($p);

// float
$x = 1224520.0;
$y = 6621300.0;
$p = proj_transform_point($pj, $x, $y);
var_dump($p);

// alternative PJ creation
$srcProj = proj_create('EPSG:3857');
$tgtProj = proj_create('EPSG:4326');
$pj2 = proj_create_crs_to_crs_from_pj($srcProj, $tgtProj);
$p = proj_transform_point($pj2, $x, $y);
var_dump($p);

// UTM to GK transformation tests
$x = 673829;
$y = 5682392;

// with proj-def
echo "\nwith proj-def\n";
$proj_utm32 = "+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs";
$proj_gk4 = "+proj=tmerc +lat_0=0 +lon_0=12 +k=1 +x_0=4500000 +y_0=0 +ellps=bessel +datum=potsdam +nadgrids=/usr/bin/BETA2007.gsb +units=m +no_defs";
$PROJ = proj_create_crs_to_crs($proj_utm32, $proj_gk4);
//echo proj_get_def($PROJ) . "\n";
//proj=pipeline step inv proj=utm zone=32 ellps=GRS80 step proj=tmerc lat_0=0 lon_0=12 k=1 x_0=4500000 y_0=0 ellps=bessel
$transformed = proj_transform_point($PROJ, $x, $y);
echo "x: " . $transformed['x'] . "\n";
echo 'y: ' . $transformed['y'] . "\n\n";

// with EPSG
echo "with EPSG\n";
$proj_gk4_epsg = proj_create('EPSG:31468');
$proj_utm32_epsg = proj_create('EPSG:25832');
$PROJ2 = proj_create_crs_to_crs_from_pj($proj_utm32_epsg, $proj_gk4_epsg);
$transformed2 = proj_transform_point($PROJ2, $x, $y);
//proj=pipeline step inv proj=utm zone=32 ellps=GRS80 step inv proj=hgridshift grids=de_adv_BETA2007.tif step proj=tmerc lat_0=0 lon_0=12 k=1 x_0=4500000 y_0=0 ellps=bessel
//echo proj_get_def($PROJ2) . "\n";
echo 'x: ' . $transformed2['x'] . "\n";
echo 'y: ' . $transformed2['y'] . "\n\n";

// over WGS84 with proj-def
echo "over WGS84 with proj-def\n";
$PROJ3 = proj_create_crs_to_crs($proj_utm32, $proj_wgs84);
$transformed3 = proj_transform_point($PROJ3, $x, $y);
$PROJ3 = proj_create_crs_to_crs($proj_wgs84, $proj_gk4);
$transformed3 = proj_transform_point($PROJ3, $transformed3['x'], $transformed3['y']);
#echo proj_get_def($PROJ3) . "\n";
#proj=pipeline step proj=unitconvert xy_in=deg xy_out=rad step inv proj=hgridshift grids=/usr/bin/BETA2007.gsb step proj=tmerc lat_0=0 lon_0=12 k=1 x_0=4500000 y_0=0 ellps=bessel
echo 'x: ' . $transformed3['x'] . "\n";
echo 'y: ' . $transformed3['y'] . "\n\n";

// over WGS84 with proj-def but without nadgrid
echo "over WGS84 with proj-def but without nadgrid\n";
$PROJ3b = proj_create_crs_to_crs($proj_utm32, $proj_wgs84);
$transformed4 = proj_transform_point($PROJ3b, $x, $y);
$proj_gk4b = "+proj=tmerc +lat_0=0 +lon_0=12 +k=1 +x_0=4500000 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs";
$PROJ3b = proj_create_crs_to_crs($proj_wgs84, $proj_gk4b);
$transformed4 = proj_transform_point($PROJ3b, $transformed4['x'], $transformed4['y']);
#echo proj_get_def($PROJ3b) . "\n";
#proj=pipeline step proj=unitconvert xy_in=deg xy_out=rad step inv proj=hgridshift grids=de_adv_BETA2007.tif step proj=tmerc lat_0=0 lon_0=12 k=1 x_0=4500000 y_0=0 ellps=bessel
echo 'x: ' . $transformed4['x'] . "\n";
echo 'y: ' . $transformed4['y'] . "\n\n";

// over WGS84 with EPSG
echo "over WGS84 with EPSG\n";
$proj_wgs84_epsg = proj_create('EPSG:4326');
$PROJ4 = proj_create_crs_to_crs_from_pj($proj_utm32_epsg, $proj_wgs84_epsg);
$transformed5 = proj_transform_point($PROJ4, $x, $y);
$PROJ4 = proj_create_crs_to_crs_from_pj($proj_wgs84_epsg, $proj_gk4_epsg);
$transformed5 = proj_transform_point($PROJ4, $transformed5['x'], $transformed5['y']);
#echo proj_get_def($PROJ4) . "\n";
#proj=pipeline step proj=unitconvert xy_in=deg xy_out=rad step inv proj=hgridshift grids=de_adv_BETA2007.tif step proj=tmerc lat_0=0 lon_0=12 k=1 x_0=4500000 y_0=0 ellps=bessel
echo 'x: ' . $transformed5['x'] . "\n";
echo 'y: ' . $transformed5['y'] . "\n\n";

try {
    proj_transform_point();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_point($pj);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_point($pj, $x);
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_point(null, $x, $y);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_point($pj, null, $y);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_transform_point($pj, $x, []);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}
?>
--EXPECTF--
resource(%d) of type (Proj)
array(3) {
  ["x"]=>
  float(11.%d)
  ["y"]=>
  float(51.%d)
  ["z"]=>
  float(%f)
}
array(3) {
  ["x"]=>
  float(11.%d)
  ["y"]=>
  float(51.%d)
  ["z"]=>
  float(%f)
}
array(3) {
  ["x"]=>
  float(11.%d)
  ["y"]=>
  float(51.%d)
  ["z"]=>
  float(%f)
}

with proj-def
x: 4464529.7463786
y: 5681254.5474169

with EPSG
x: 4464630.4985116
y: 5681395.7173844

over WGS84 with proj-def
x: 4464630.4985116
y: 5681395.7173844

over WGS84 with proj-def but without nadgrid
x: 4464630.4985116
y: 5681395.7173844

over WGS84 with EPSG
x: 4464630.4985116
y: 5681395.7173844

proj_transform_point() expects at least 3 arguments, 0 given
proj_transform_point() expects at least 3 arguments, 1 given
proj_transform_point() expects at least 3 arguments, 2 given
proj_transform_point(): Argument #1 ($Proj) must be of type resource, null given

Deprecated: proj_transform_point(): Passing null to parameter #2 ($x) of type float is deprecated in %s
proj_transform_point(): Argument #3 ($y) must be of type float, array given
