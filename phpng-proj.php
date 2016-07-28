<?php
$br = (php_sapi_name() == "cli")? "":"<br>";
if(!extension_loaded('proj4')) {
	die("Extension proj4 not available.");
}
if (extension_loaded("proj4")) {
	echo "Module proj4 is compiled into PHP";
}
$module = 'proj4';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
