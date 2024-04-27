SKIPUNZIP=1

FLAVOR=@FLAVOR@
MODULE_ID=@MODULE_ID@

TMP_MODULE_DIR=/data/local/tmp/re.zyg.fri

if [ "$FLAVOR" != "zygisk" ] && [ "$FLAVOR" != "riru" ]; then
  abort "! Unknown ZygiskFrida flavor: $FLAVOR"
else
  ui_print "- ZygiskFrida flavor: $FLAVOR"
fi

if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print    "*********************************************************"
  ui_print    "! Unable to extract verify.sh!"
  ui_print    "! This zip may be corrupted, please try downloading again"
  abort "*********************************************************"
fi
. $TMPDIR/verify.sh

ui_print "- Extracting module files"
extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'uninstall.sh' "$MODPATH"

if [ "$FLAVOR" = "riru" ]; then
  ui_print "- Extracting riru.sh"
  extract "$ZIPFILE" 'riru.sh' "$TMPDIR"
  . $TMPDIR/riru.sh

  check_riru_version
  enforce_install_from_magisk_app
fi

LIB32_NAME="armeabi-v7a.so"
LIB64_NAME="arm64-v8a.so"
LIB32_DEST="$MODPATH/zygisk"
LIB64_DEST="$MODPATH/zygisk"
BUSYBOX_BIN=/data/adb/magisk/busybox

if [ ! -f $BUSYBOX_BIN ]; then
  BUSYBOX_BIN=/data/adb/ksu/bin/busybox
fi

if [ ! -f $BUSYBOX_BIN ]; then
  abort "! unable to locate busybox"
fi

ui_print "- Using busybox: $BUSYBOX_BIN"

[ "$FLAVOR" = "riru" ] && LIB32_DEST="$MODPATH/riru/lib"
[ "$FLAVOR" = "riru" ] && LIB64_DEST="$MODPATH/riru/lib64"
[ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ] && LIB32_NAME="x86.so"
[ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ] && LIB64_NAME="x86_64.so"

mkdir -p "$LIB32_DEST"
mkdir -p "$LIB64_DEST"

ui_print "- Extracting 32-bit libraries"
extract "$ZIPFILE" "lib/$LIB32_NAME" "$LIB32_DEST" true
[ "$FLAVOR" = "riru" ] && mv "$LIB32_DEST/$LIB32_NAME" "${LIB32_DEST}/lib${MODULE_ID}.so"

if [ "$IS64BIT" = true ]; then
  ui_print "- Extracting 64-bit libraries"
  extract "$ZIPFILE" "lib/$LIB64_NAME" "$LIB64_DEST" true
  [ "$FLAVOR" = "riru" ] && mv "$LIB64_DEST/$LIB64_NAME" "${LIB64_DEST}/lib${MODULE_ID}.so"
fi

ui_print "- Extracting bundled frida gadget"

mkdir -p "$TMP_MODULE_DIR"
extract "$ZIPFILE" "gadget/libgadget-$ARCH.so.xz" "$TMP_MODULE_DIR" true
mv "$TMP_MODULE_DIR/libgadget-$ARCH.so.xz" "$TMP_MODULE_DIR/libgadget.so.xz"
rm "$TMP_MODULE_DIR/libgadget.so"
$BUSYBOX_BIN unxz "$TMP_MODULE_DIR/libgadget.so.xz"
rm "$TMP_MODULE_DIR/libgadget.so.xz"

if [ "$IS64BIT" = true ]; then
  ARCH32="arm"
  [ "$ARCH" = "x64" ] && ARCH32="x86"

  extract "$ZIPFILE" "gadget/libgadget-$ARCH32.so.xz" "$TMP_MODULE_DIR" true
  mv "$TMP_MODULE_DIR/libgadget-$ARCH32.so.xz" "$TMP_MODULE_DIR/libgadget32.so.xz"
  rm "$TMP_MODULE_DIR/libgadget32.so"
  $BUSYBOX_BIN unxz "$TMP_MODULE_DIR/libgadget32.so.xz"
  rm "$TMP_MODULE_DIR/libgadget32.so.xz"
fi

extract "$ZIPFILE" "config.json.example" "$TMP_MODULE_DIR" true

set_perm_recursive "$TMP_MODULE_DIR" 0 0 0755 0644
set_perm_recursive "$MODPATH" 0 0 0755 0644





