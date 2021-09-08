--TEST--
proj_strerrno() function - basic test for proj_strerrno()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
#$pj = proj_create('foo');
#$ref = proj_get_errno($pj);
var_dump(proj_get_errno_string(-1));
#unset($pj);
?>
--EXPECT--
string(35) "no arguments in initialization list"