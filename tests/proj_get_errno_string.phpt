--TEST--
proj_get_errno_string() function - basic test for proj_get_errno_string()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
if (proj_get_info()['major']>7) {
    var_dump(proj_get_errno_string(1024));
} else {
    var_dump(proj_get_errno_string(-1));
}
#unset($pj);
?>
--EXPECTF--
string(%d) "%s"