# Simple Config

Please consider using the [structured config](advanced_config.md) instead.

This file is about the previous configuration method via different files
using `target_packages` and `injected_libraries` and doesn't support all the
features.

`/data/local/tmp/re.zyg.fri/target_packages` is a simple text file
containing all the package names you want to inject frida into.

It accepts one package name per line f.e.
```
adb shell 'su -c "echo com.example.package > /data/local/tmp/re.zyg.fri/target_packages"'
```

**Start up delay**

There are times that you might want to delay the injection of the gadget. Some applications
might run checks at start up and delaying the injection can help avoid these.

`/data/local/tmp/re.zyg.fri/target_packages` accepts a start up delay in milliseconds.
You can provide it separated by a comma from the package_name.

f.e.
```
adb shell 'su -c "echo com.example.package,20000 > /data/local/tmp/re.zyg.fri/target_packages"'
```
would inject the gadget after a delay of 20 seconds.

You get a 10 seconds countdown to injection in the ZygiskFrida logs `adb logcat -S ZygiskFrida`.
This can help if you want to time the injection with app interactions.

**Gadget version and config**

The bundled gadget is located at `/data/local/tmp/re.zyg.fri/libgadget.so`.\
You can follow the [Gadget Docs](https://frida.re/docs/gadget/) and add additional
gadget config and scripts in that location.

In case you want to use a different gadget version than the one bundled, you can simply
replace the `libgadget.so` with your own frida gadget.

**Loading arbitrary libraries**

This module also allows you to load arbitrary .so libraries into the process.\
This can allow you to load additional helper libraries for the gadget or
enable any other use case that might need libraries loaded into the app process.

For this you can add the file `/data/local/tmp/re.zyg.fri/injected_libraries`.\
The file should consist of file paths to libraries.
The libraries are loaded in the order they are specified in the file.

Example file content that would first load libhelperexample.so and then the bundled frida-gadget:
```
/data/local/tmp/re.zyg.fri/libhelperexample.so
/data/local/tmp/re.zyg.fri/libgadget.so
```

Make sure the libraries are located somewhere accessible by the app and that
file permissions are properly set.

If you want the frida gadget to start, you need to explicitly specify the bundled frida-gadget at
`/data/local/tmp/re.zyg.fri/libgadget.so`.\
You can also choose to specify your own gadget this way or omit the gadget altogether.