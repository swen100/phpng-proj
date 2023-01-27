<?php

$br = php_sapi_name() == "cli" ? PHP_EOL : "<br>" . PHP_EOL;

$module = 'proj';
if(!extension_loaded($module)) {
    die("Extension $module not available.$br");
}

if (extension_loaded($module)) {
    echo "Module $module is compiled into PHP.$br";
}

$functions = get_extension_funcs($module);
echo "Functions available in this extension:$br";
foreach($functions as $func) {
    echo "\t- " . $func . "()$br";
}

echo "<pre>";
    print_r(proj_list_units());
    print_r(proj_list_ellps());
echo "</pre>";
