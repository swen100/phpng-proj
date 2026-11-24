--TEST--
proj_list_units() / proj_list_ellps() - basic test
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$units = proj_list_units();
var_dump(is_array($units) && count($units) > 0);
$first = reset($units);
var_dump(is_array($first) && isset($first['name']));

$ellps = proj_list_ellps();
var_dump(isset($ellps['WGS84']), $ellps['WGS84']['name']);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
string(6) "WGS 84"
