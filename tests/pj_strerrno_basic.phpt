--TEST--
pj_strerrno() function - basic test for pj_strerrno()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = pj_init_plus('foo');
$ref = pj_get_errno_ref();
var_dump(pj_strerrno($ref));
unset($pj);
?>
--EXPECT--
string(35) "no arguments in initialization list"