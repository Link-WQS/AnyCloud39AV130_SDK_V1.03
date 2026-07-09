obj-$(CONFIG_LZ4_COMPRESS) += lz4_compress.o
obj-$(CONFIG_LZ4HC_COMPRESS) += lz4hc_compress.o
obj-$(CONFIG_LZ4_DECOMPRESS) += lz4_decompress.o
ifeq ($(CONFIG_SYS_FAST_LAUNCH),y)
obj-$(CONFIG_LZ4_DECOMPRESS) += lz5_decompress.o
endif