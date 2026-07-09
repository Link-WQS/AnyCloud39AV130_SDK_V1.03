#!/bin/sh

mkdir -p tmpfs
cp -af rootfs/* tmpfs
rm -rf tmpfs/usr/* tmpfs/etc/config/*

rm -rf usr.fat32
mkfs.vfat -n "config" -F 32 -C usr.fat32 1024
mcopy -/ -i usr.fat32 rootfs/etc/config/* ::/


rm -rf tmpfs

