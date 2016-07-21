# phpng-proj

A php extension for proj.4

## API Functions

### Basic API

**resource pj_init_plus(string definition);**
Create a Proj.4 resource coordinate system object from the string definition.

**array pj_transform_point(resource srcdefn, resource dstdefn, mixed x, mixed y, mixed z);**
Transform the x/y/z points from the source coordinate system to the destination coordinate system.
x, y and z can be double, int or a numeric string.

~~**array pj_transform(resource srcdefn, resource dstdefn, int point_count, int point_offset, mixed x, mixed y, mixed z);**
Transform the x/y/z points from the source coordinate system to the destination coordinate system.
x, y and z can be double, int or a numeric string.
x, y and z can also be an array of double, int or numeric string values.~~

_The function "pj_transform" was replaces by pj_transform_string and pj_transform_array._

**array pj_transform_string(resource srcdefn, resource dstdefn, string xyz);**
Transform the x/y[/z] string from the source coordinate system to the destination coordinate system.

**array pj_transform_array(resource srcdefn, resource dstdefn, array xyz);**
Transform the x/y[/z] array from the source coordinate system to the destination coordinate system.
The array can contain strings with x,y,z-values or also an array with x,y[,z]-values where x, y and z can be of type double, int or numeric string.

**void pj_free(resource pj);**
Frees all resources associated with pj.

### Advanced Functions

**boolean pj_is_latlong(resource pj);**
Returns true if the passed coordinate system is geographic (proj=latlong).
  
**boolean pj_is_geocent(resource pj);**
Returns true if the coordinate system is geocentric (proj=geocent).

**string pj_get_def(resource pj, int options);**
Returns the PROJ.4 initialization string suitable for use with pj_init_plus() that would produce this coordinate system, but with the definition expanded as much as possible (for instance +init= and +datum= definitions).

**resource proj_wgs84_from_proj(resource pj_in);**
Returns a new coordinate system definition which is the geographic coordinate (lat/long) system underlying pj_in.

### Environment Functions

**void pj_deallocate_grids();**
Frees all resources associated with loaded and cached datum shift grids.

**string pj_strerrno(int);**
Returns the error text associated with the passed in error code.

**int pj_get_errno_ref();**
Returns an integer value that can be used for the pj_strerrno(int) function.

**string pj_get_release();**
Returns an internal string describing the release version.

## Code examples

```php
<?php  
	$proj_wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	$proj_merc = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
?>
```

### example 1:
Transforming variables with x and y [and z] values.
```php
<?php  
	if ($proj_merc !== false && $proj_wgs84 !== false) {  
	    $x = 1224526;
	    $y = 6621326;
	    $transformed = pj_transform_point($proj_merc, $proj_wgs84, $x, $y);  
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
	if ($proj_merc !== false && $proj_wgs84 !== false) {  
	    $coords = "11 51,11.5 51.5 20";
	    $transformed = pj_transform_string($proj_wgs84, $proj_merc, $coords);  
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
	if ($proj_merc !== false && $proj_wgs84 !== false) {  
	    $coords = ["11 51", "11.5 51.5 20"];
	    $transformed = pj_transform_array( $proj_wgs84, $proj_merc, $coords );
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
	if ($proj_merc !== false && $proj_wgs84 !== false) {  
	    $coords = [[11, 51], [11.5, 51.5, 20]];
	    $transformed = pj_transform_array( $proj_wgs84, $proj_merc, $coords );
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
