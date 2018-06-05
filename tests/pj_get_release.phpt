--TEST--
proj_get_release() function - basic test for proj_get_release()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
var_dump(proj_get_release());
?>
--EXPECTREGEX--
string\([\d]+\) \"Rel\. [\d]+\.[\d]+\.[\d]+, [a-zA-Z]+ [\d]+[a-z]+, [\d]+\"
