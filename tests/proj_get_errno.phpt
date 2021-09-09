--TEST--
proj_get_errno() function - basic test for proj_get_errno()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php

$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832');
var_dump(proj_get_errno($pj));
?>
--EXPECT--
int(0)