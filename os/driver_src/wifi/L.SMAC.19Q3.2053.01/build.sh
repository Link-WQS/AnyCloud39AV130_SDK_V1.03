SSV_DRV_PATH=absolute_path_to_driver_source_folder
KVERSION="`uname -r`"
make -C /lib/modules/$KVERSION/build SSV_DRV_PATH=$SSV_DRV_PATH M=$SSV_DRV_PATH clean
make -C /lib/modules/$KVERSION/build SSV_DRV_PATH=$SSV_DRV_PATH M=$SSV_DRV_PATH modules
make -C $SSV_DRV_PATH strip
