SKIPUNZIP=1

FLAVOR=@FLAVOR@

TMP_MODULE_DIR=/data/local/tmp/re.zyg.fri

if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

extract() {
  zip=$1
  file=$2
  dir=$3

  unzip -oj "$zip" "$file" -d "$dir" >&2
}

mkdir -p "$MODPATH/zygisk"
mkdir -p "$TMP_MODULE_DIR"
extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'uninstall.sh' "$MODPATH"

if [ "$ARCH" = "arm" ]  || [ "$ARCH" = "arm64" ]; then
  ui_print "- Extracting arm libraries"
  extract "$ZIPFILE" "zygisk/armeabi-v7a.so" "$MODPATH/zygisk"

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting arm64 libraries"
    extract "$ZIPFILE" "zygisk/arm64-v8a.so" "$MODPATH/zygisk"
    extract "$ZIPFILE" "gadget/libgadget-arm64.so.xz" "$TMP_MODULE_DIR"
    mv "$TMP_MODULE_DIR/libgadget-arm64.so.xz" "$TMP_MODULE_DIR/libgadget.so.xz"
  else
    extract "$ZIPFILE" "gadget/libgadget-arm.so.xz" "$TMP_MODULE_DIR"
    mv "$TMP_MODULE_DIR/libgadget-arm.so.xz" "$TMP_MODULE_DIR/libgadget.so.xz"
  fi
fi

if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
  ui_print "- Extracting x86 libraries"
  extract "$ZIPFILE" "zygisk/x86.so" "$MODPATH/zygisk"

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting x64 libraries"
    extract "$ZIPFILE" "zygisk/x86_64.so" "$MODPATH/zygisk"
    extract "$ZIPFILE" "gadget/libgadget-x64.so.xz" "$TMP_MODULE_DIR"
    mv "$TMP_MODULE_DIR/libgadget-x64.so.xz" "$TMP_MODULE_DIR/libgadget.so.xz"
  else
    extract "$ZIPFILE" "gadget/libgadget-x86.so.xz" "$TMP_MODULE_DIR"
    mv "$TMP_MODULE_DIR/libgadget-x86.so.xz" "$TMP_MODULE_DIR/libgadget.so.xz"
  fi
fi

/data/adb/magisk/busybox unxz "$TMP_MODULE_DIR/libgadget.so.xz"
rm "$TMP_MODULE_DIR/libgadget.so.xz"

touch "$TMP_MODULE_DIR/target_packages"

set_perm_recursive "$TMP_MODULE_DIR" 0 0 0755 0644
set_perm_recursive "$MODPATH" 0 0 0755 0644





