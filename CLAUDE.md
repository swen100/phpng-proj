# CLAUDE.md

PHP C extension (`proj`) wrapping libPROJ 6+ for PHP 7.0–8.x. Exposes coordinate transformation functions (`proj_create*`, `proj_transform_*`, `proj_distance_*`, …) to PHP userland.

## Layout

- `src/proj.c` – the whole extension: arginfo, `ZEND_FUNCTION`s, function table (`proj_functions[]`), resource destructors, MINIT/MINFO.
- `src/php_proj.h` – function declarations, `PHP_PROJ_VERSION`, resource names (`Proj`, `Area`).
- `config.m4` (Linux, `--with-proj`) / `config.w32` (Windows) – build config.
- `tests/*.phpt` – PHP test files, one per function.
- `phpng-proj.php` – smoke script: lists the extension's functions (run in CI after tests).
- `package.xml` (PECL), `composer.json` – packaging metadata.

## Build & test (Linux, needs `libproj-dev` + php-dev)

```bash
phpize && ./configure && make
make test                                   # all tests
make test TESTS=tests/proj_create.phpt      # one test
php -d extension=modules/proj.so phpng-proj.php
make clean && phpize --clean                # clean up
```

CI: `.travis.yml` (PHP 7.0–8.0, Ubuntu jammy).

## Adding a PHP function

Touch all four places, or the function won't exist:
1. `ZEND_FUNCTION(name);` in `src/php_proj.h`
2. `ZEND_BEGIN_ARG_INFO(name_args, …)` + `ZEND_FUNCTION(name)` in `src/proj.c`
3. `ZEND_FE(name, name_args)` in `proj_functions[]`
4. `tests/name.phpt` and an entry in the README API list

## Conventions & gotchas

- PJ objects are PHP resources (`Proj`, `Area`), freed by `php_proj_dtor` / `php_proj_area_dtor`. Never `proj_destroy` a PJ that is still owned by a resource.
- `proj4_*` functions are the legacy API (take two proj strings); `proj_*` take a `Proj` resource from `proj_create_crs_to_crs*`. Keep both working.
- Must compile against PHP 7.0 through 8.x and PROJ 6 through current; guard version-specific code with `PHP_VERSION_ID` / `PROJ_VERSION_MAJOR`.
- `.phpt` expectations include exact PHP error/deprecation messages, which differ between PHP versions; use `--EXPECTF--` with `%d`/`%f`/`%s` where values vary.
- Version number lives in `PHP_PROJ_VERSION` (`src/php_proj.h`) and `package.xml`; bump both on release.
