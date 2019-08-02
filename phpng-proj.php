<?php
$br = (php_sapi_name() == "cli")? "":"<br>";
$module = 'proj';
if(!extension_loaded($module)) {
	die("Extension $module not available.");
}
if (extension_loaded($module)) {
	echo "Module $module is compiled into PHP$br\n";
}
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
