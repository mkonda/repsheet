# Repsheet

## Apache Module

To build the module you can just run `make`. It will ensure that the module builds properly on your system. If you are running OS X, you might need to run `make mountain_lion_setup` to symlink the proper dev tools into place for apxs.

#### Installing

Just run `sudo make install`. It uses apxs to compile and install the module. You just need to enable the module using the `RepsheetEnabled` directive in your apache/httpd.conf file

```
RepsheetEnabled On
```
