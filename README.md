# ZygiskFrida

> [Frida](https://frida.re) is a dynamic instrumentation toolkit for developers, reverse-engineers, and security researchers

> [Zygisk](https://github.com/topjohnwu/Magisk) part of Magisk allows you to run code in every Android application's Process.


## Introduction

[ZygiskFrida](README.md) is a zygisk module allowing you to inject frida gadget in Android applications in a
more stealthy way.

- The gadget is not embedded into the APK itself. So APK Integrity/Signature checks will still pass.
- The process is not being ptraced like it is with frida-server. Avoiding ptrace based detection.

**This is an early version and things are still unstable. Deployment and usage might still change significantly.**

## How to build

There are two alternatives to build this project.

### Using Github Actions

Trigger the [Build Workflow](https://github.com/lico-n/ZygiskFrida/actions/workflows/build.yml) via workflow dispatch.

After you triggered the build, wait for the build to complete.
The magisk module zip file will be an output artifact of that build.

Parameters that you have to specify:

**Package Name**\
The gadget will only be injected in processes with this package name.

f.e. com.package.example

**Gadget Download link**\
Visit the [frida release page](https://github.com/frida/frida/releases).
Get the download link for the correct architecture of your device.

f.e. https://github.com/frida/frida/releases/download/16.1.3/frida-gadget-16.1.3-android-arm64.so.xz




### Checking out the project

Building the project yourself allows you to rename things making it more stealthy.\
It also allows you to add files like a gadget config and scripts, see [Gadget Doc](https://frida.re/docs/gadget/)

- Checkout the project
- Initialize the submodules with `git submodule update --init`
- The `./gadget` directory will be transferred to the device.\
  You must at least put the uncompressed gadget library in here.\
  You can download it from the [frida release page](https://github.com/frida/frida/releases) and extract it into this directory.
  Here you can also put additional gadget config/scripts.
- Update `./module.gradle`.\
  `appPackageName` is the process name you want to inject frida into.\
  `gadgetPath` is the path relative to the `./gadget` directory. f.e. \
   if you put the gadget `./gadget/frida-gadget-16.1.3-android-arm64.so` here,\
   then specify `frida-gadget-16.1.3-android-arm64.so`
- Run `./gradlew :module:assembleRelease`
- The build magisk module should then be in the `out` directory.

## How to use the module

- Transfer the module zip file to your device and install it via Magisk.
- Reboot after install
- Launch your app. It will pause at startup allowing you to attach
  f.e. `frida -U -N com.example.package` or `frida -U -n Gadget`

The first launch after install might not work. In that case please try to force close the app and retry.

You can change the package that is injected without rebuilding by
replacing the text file at `/data/adb/modules/zygiskfrida/target_packages`.\
The text file should contain the package names you want the gadget to start in separated by newlines.

## Caveats

- This module will transfer the gadget files into the `/data/app/**/{app_package_name}/lib/{arch}/re.zyg.fri` directory at app startup.\
  The files will remain there as leftovers. You can free up device space by deleting them nanually.

- For emulators this will start the gadget in native realm. This means that you will be able to hook Java but not native functions.\
  Choose the gadget of the architecture of your host instead of the emulated phone f.e. `frida-gadget-16.1.3-android-x86_64.so`.

- This is not yet tested very well on different devices.\
  In case this is not working reports with logs `adb logcat -S ZygiskFrida` are welcome.


## Credits

- Inspired by https://github.com/Perfare/Zygisk-Il2CppDumper
