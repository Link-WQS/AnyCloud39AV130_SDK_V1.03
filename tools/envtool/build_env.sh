rm env.img
tr '\000' '\377' < /dev/zero | dd of=./env.img bs=1024 count=4
./fw_setenv -s env_av130_64M_spinor.cfg
./fw_printenv
cp -rf env.img  ../burntool/env_av130_64M_spinor.img
cp -rf env.img  ../../image/env_av130_64M_spinor.img