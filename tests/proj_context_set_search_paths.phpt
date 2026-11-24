--TEST--
proj_context_set_search_paths() function - set and reset the PROJ search paths
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
} elseif (!function_exists('proj_context_set_search_paths')) {
    echo 'skip proj_context_set_search_paths() not available';
}
?>
--FILE--
<?php
// PROJ trennt die Suchpfade plattformabhaengig
$separator = (DIRECTORY_SEPARATOR === '\\') ? ';' : ':';

$default = proj_get_info()['searchpath'];
$paths = [sys_get_temp_dir(), __DIR__];

// explizit setzen
var_dump(proj_context_set_search_paths($paths));
var_dump(proj_get_info()['searchpath'] === implode($separator, $paths));

// leeres Array setzt auf die Standardpfade zurueck
var_dump(proj_context_set_search_paths([]));
var_dump(proj_get_info()['searchpath'] === $default);

// nach dem Reset wird proj.db wieder gefunden
var_dump(is_resource(proj_create('EPSG:4326')));

// Nicht-String-Eintraege: Warnung, keine Aenderung
var_dump(proj_context_set_search_paths([sys_get_temp_dir(), 42]));
var_dump(proj_get_info()['searchpath'] === $default);

try {
    proj_context_set_search_paths(sys_get_temp_dir());
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    proj_context_set_search_paths();
} catch (ArgumentCountError $exc) {
    echo $exc->getMessage() . "\n";
}
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

Warning: proj_context_set_search_paths(): All search paths must be strings in %s on line %d
bool(false)
bool(true)
proj_context_set_search_paths(): Argument #1 ($paths) must be of type array, string given
proj_context_set_search_paths() expects exactly 1 argument, 0 given
