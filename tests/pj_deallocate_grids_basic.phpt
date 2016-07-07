--TEST--
pj_deallocate_grids() function - basic test for pj_deallocate_grids()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) print 'skip proj.4 extension not available';
?>
--FILE--
<?php
var_dump(pj_deallocate_grids());
?>
--EXPECT--
NULL