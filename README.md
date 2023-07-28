# ZygiskFrida

> [Frida](https://frida.re) is a dynamic instrumentation toolkit for developers, reverse-engineers, and security researchers

> [Zygisk](https://github.com/topjohnwu/Magisk) part of Magisk allows you to run code in every Android application's Process.


## Introduction

[ZygiskFrida](README.md) is a zygisk module allowing you to inject frida gadget in Android applications in a
more stealthy way.

- The gadget is not embedded into the APK itself. So APK Integrity/Signature checks will still pass.
- The process is not being ptraced like it is with frida-server. Avoiding ptrace based detection.

## How to use the module

- Download the latest release from the [Release Page](https://github.com/lico-n/ZygiskFrida/releases)
- Transfer the ZygiskFrida zip file to your device and install it via Magisk.
- Reboot after install
- Update `/data/local/tmp/re.zyg.fri/target_packages` on your device with the target package names.\
  Apps with matching package names will be injected with the gadget. One package name per line.\
  f.e. `adb shell 'su -c "echo com.example.package > /data/local/tmp/re.zyg.fri/target_packages"'`

- Launch your app. It will pause at startup allowing you to attach
  f.e. `frida -U -N com.example.package` or `frida -U -n Gadget`

The gadget is located at `/data/local/tmp/re.zyg.fri/libgadget.so`.\
You can follow the [Gadget Docs](https://frida.re/docs/gadget/) to add additional
gadget config and scripts into that location.

In case you want to use a different gadget version than the one bundled, you can simply
replace the `libgadget.so` with your own frida gadget.

## How to build

Building the project yourself allows you to rename things making it more stealthy.

- Checkout the project
- Run `./gradlew :module:assembleRelease`
- The build magisk module should then be in the `out` directory.

You can also build and install the module with `./gradlew :module:flashAndRebootRelease`

## Caveats

- For emulators this will start the gadget in native realm. This means that you will be able to hook Java but not native functions.

- This is not yet tested very well on different devices.\
  In case this is not working reports with logs `adb logcat -S ZygiskFrida` are welcome.


## Credits

- Inspired by https://github.com/Perfare/Zygisk-Il2CppDumper
- https://github.com/hexhacking/xDL

