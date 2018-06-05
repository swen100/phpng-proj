--TEST--
pj_init_plus() function - error test for pj_init_plus()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = pj_init_plus('+proj=foo');
var_dump($pj);
?>
--EXPECT--
bool(false)