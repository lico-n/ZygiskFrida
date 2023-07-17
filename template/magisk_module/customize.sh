TMP_MODULE_DIR=/data/local/tmp/${modulePackageName}

mkdir -p \$TMP_MODULE_DIR

cp \$MODPATH/gadget/libgadget-\$ARCH.so.xz \$TMP_MODULE_DIR/${gadgetLibraryName}.xz
/data/adb/magisk/busybox unxz \$TMP_MODULE_DIR/${gadgetLibraryName}.xz
touch \$TMP_MODULE_DIR/target_packages

set_perm_recursive \$TMP_MODULE_DIR 0 0 0755 0644
