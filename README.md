# phpng-proj6+

A php extension to be usable with PHP7+ and next generation of libProj (6+).

[![Build Status](https://travis-ci.com/swen100/phpng-proj.svg?branch=proj6+)](https://travis-ci.com/swen100/phpng-proj)

### Installation

[see INSTALL.md](INSTALL)

## API Functions

### Basic API

**resource proj_create(string definition);**
Create a Proj resource coordinate system object from the projection definition suitable for use with proj_create_crs_to_crs_from_pj(). Be aware: the definitions used for proj_create have then to be in form of "EPSG:xxx"!

**resource proj_create_crs_to_crs(string srcCrs, string tgtCrs);**
Create a Proj resource coordinate system object from the source SRID and target SRID.

**resource proj_create_crs_to_crs_from_pj(resource srcPj, resource tgtPj);**
Create a Proj resource coordinate system object from the source PROJ and target PROJ.

**array proj_transform_point(resource pj, mixed x, mixed y, mixed z);**
Transform the x/y/z points from the previously set coordinate systems (source, target) within projection.
x, y and z can be double, int or a numeric string.

**array proj_transform_string(resource pj, string xyz);**
Transform the x/y[/z] string from the previously set coordinate systems (source, target) within projection.

**array proj_transform_array(resource pj, array xyz);**
Transform the x/y[/z] array from the previously set coordinate systems (source, target) within projection.
The array can contain strings with x,y,z-values or also an array with x,y[,z]-values where x, y and z can be of type double, int or numeric string.

**void proj_destroy(resource pj);**
Frees all resources associated with pj.

### Advanced Functions

**boolean proj_is_latlong(resource pj);**
Returns true if the output coordinate system is geographic (proj=latlong).

**boolean proj_is_geocent(resource pj);**
Returns true if the output coordinate system is geocentric (proj=geocent).

**string proj_get_def(resource pj);**
Returns the PROJ initialization string suitable for use with proj_create() that would produce this coordinate system, but with the definition expanded as much as possible (for instance +init= and +datum= definitions).

### Environment Functions

**string proj_get_errno_string(int);**
Returns the error text associated with the passed in error code.

**int proj_get_errno(resource pj);**
Returns an integer value that can be used for the proj_errno_string(int) function.

**array proj_get_info();**
Returns an array with informations about the current PROJ library.

**string proj_get_release();**
Returns an internal string describing the release version.

## Code examples

```php
<?php  
	$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
	$proj_merc = "+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs";
        $PROJ = proj_create_crs_to_crs($proj_wgs84, $proj_merc);
?>
```

### example 1:
Transforming variables with x and y [and z] values.
```php
<?php  
	if ($PROJ !== false) {  
	    $x = 1224526;
	    $y = 6621326;
	    $transformed = proj_transform_point($PROJ, $x, $y);  
	    print 'latitude: '.$transformed['x'].'<br />';  
	    print 'longitude: '.$transformed['y'].'<br />';  
	}
?>
```

**Output:**
```
	latitude: 11.000104216017
	longitude: 51.000182472069
```

### example 2:
Transforming a string containing x,y[,z]-values.
```php
<?php  
	if ($PROJ !== false) {  
	    $coords = "11 51,11.5 51.5 20";
	    $transformed = proj_transform_string($PROJ, $coords);  
	    print_r($transformed);  
	}
?>
```

**Output:**
```
	Array
        (
            [0] => Array
                (
                    [x] => 1224514.398726
                    [y] => 6621293.7227402
                    [z] => 0
                )

            [1] => Array
                (
                    [x] => 1280174.1441226
                    [y] => 6710219.0832207
                    [z] => 20
                )

        )
```

### example 3:
Transforming an array containing x,y[,z]-values as strings.
```php
<?php  
	if ($PROJ !== false) {  
	    $coords = ["11 51", "11.5 51.5 20"];
	    $transformed = proj_transform_array( $PROJ, $coords );
	    print_r($transformed);  
	}
?>
```

**Output:**
```
	Array
	(
	    [0] => Array
	        (
	            [x] => 1224514.398726
	            [y] => 6621293.7227402
	            [z] => 0
	        )
	
	    [1] => Array
	        (
	            [x] => 1280174.1441226
	            [y] => 6710219.0832207
	            [z] => 20
	        )
	
	)
```

### example 4:
Transforming an array containing x,y[,z]-values as arrays.
```php
<?php  
	if ($PROJ !== false) {  
	    $coords = [[11, 51], [11.5, 51.5, 20]];
	    $transformed = proj_transform_array( $PROJ, $coords );
	    print_r($transformed);  
	}
?>
```

**Output:**
```
	Array
	(
	    [0] => Array
	        (
	            [x] => 1224514.398726
	            [y] => 6621293.7227402
	            [z] => 0
	        )
	
	    [1] => Array
	        (
	            [x] => 1280174.1441226
	            [y] => 6710219.0832207
	            [z] => 20
	        )
	
	)
```
