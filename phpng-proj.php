<?php
$br = (php_sapi_name() == "cli")? "\n":"<br>\n";
$module = 'proj';
if(!extension_loaded($module)) {
    die("Extension $module not available.");
}
if (extension_loaded($module)) {
    echo "Module $module is compiled into PHP$br";
}
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br";
foreach($functions as $func) {
    echo $func."$br";
}


echo "<pre>";
print_r(proj_list_units());

print_r(proj_list_ellps());
