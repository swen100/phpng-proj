How to install phpng-proj extension
----------------------------------

make sure the following package is installed: libproj-dev

```bash
# Dependencies you need to install
# git-core php7-devel libproj-dev

git clone https://github.com/swen100/phpng-proj
cd phpng-proj

# if it´s not the master already
git checkout proj5+

# Build and install the extension
phpize
./configure

# this copies the proj.so file to the php-library directory eg.: /usr/lib/php7/extensions/ 
make install

# optional
make test

# Clean up the build files
make clean
phpize --clean
```

create the file 'proj.ini' in '/etc/php7/conf.d' with the following content: 'extension=proj.so'

restart your webserver or php7-fpm.
