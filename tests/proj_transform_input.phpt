--TEST--
proj_transform_*() - input is not modified, bad points give false, missing z is 0
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$pj = proj_create_crs_to_crs('EPSG:4326', 'EPSG:3857');

// dynamic (refcounted) strings and nested arrays: the caller's array must stay as it was
$x = 11; $y = 51;
$points = ["$x $y", [(string)$x, (string)$y], "  $x   $y  "];
$copy = $points;
$result = proj_transform_array($pj, $points);
var_dump($points === $copy);
var_dump(count($result), round($result[0]['x']) === round($result[1]['x']), round($result[0]['y']) === round($result[2]['y']));
unset($copy, $points);

// references inside the array
$p = [11, 51];
$ref = [&$p];
var_dump(is_array(proj_transform_array($pj, $ref)[0]));

// unreadable or failing points become false, the others are still transformed
$result = proj_transform_array($pj, ["11", [11], "", "abc def", "11 91", "11 51"]);
var_dump($result[0], $result[1], $result[2], $result[3], $result[4], is_array($result[5]));

$result = proj_transform_string($pj, "11 51,, 11 91");
var_dump(is_array($result[0]), $result[1], $result[2]);

// proj_transform_point without z gives z = 0, failing point gives false
$p = proj_transform_point($pj, 11, 51);
var_dump($p['z']);
var_dump(proj_transform_point($pj, 11, 91));

// legacy API: same rules
$wgs84 = proj_create('+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs');
$merc = proj_create('+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs');
$result = proj4_transform_array($wgs84, $merc, ["11 51", "11"]);
var_dump(is_array($result[0]), $result[1]);
$result = proj4_transform_string($wgs84, $merc, "11  51, x");
var_dump(round($result[0]['y']) > 6000000, $result[1]);
?>
--EXPECTF--
bool(true)
int(3)
bool(true)
bool(true)
bool(true)
%Abool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
%Abool(true)
bool(false)
bool(false)
float(0)
%Abool(false)
bool(true)
bool(false)
bool(true)
bool(false)
