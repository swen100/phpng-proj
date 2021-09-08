--TEST--
proj_get_info() function - basic test for proj_get_info()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
var_dump(proj_get_info());
?>
--EXPECTF--
array(6) {
  ["version"]=>
  string(5) "%d.%d.%d"
  ["major"]=>
  int(%d)
  ["minor"]=>
  int(%d)
  ["patch"]=>
  int(%d)
  ["release"]=>
  string(%d) "%s"
  ["searchpath"]=>
  string(%d) "%s"
}
