
log:
	adb logcat -s ZygiskFrida

deploy:
	.\gradlew :module:assembleRelease
	adb push ./out/zygiskfrida-v1.0.0-release.zip /sdcard/Download/
	adb shell "su -c magisk --install-module /sdcard/Download/zygiskfrida-v1.0.0-release.zip"
	adb shell "reboot"


start:
	adb shell "adb forward tcp:27052 tcp:27052"
