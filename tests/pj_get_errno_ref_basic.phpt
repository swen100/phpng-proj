--TEST--
pj_get_errno_ref() function - basic test for pj_get_errno_ref()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
pj_init_plus('+proj=foo');
var_dump(pj_get_errno_ref());
?>
--EXPECT--
int(-5)