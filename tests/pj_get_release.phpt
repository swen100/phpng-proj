--TEST--
pj_get_release() function - basic test for pj_get_release()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
var_dump(pj_get_release());
?>
--EXPECTREGEX--
string\([\d]+\) \"Rel\. [\d]+\.[\d]+\.[\d]+, [\d]+ [a-zA-Z]+ [\d]+\"