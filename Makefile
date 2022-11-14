LKM_NAME = 7_segment_display
DEV_TREE_OVERLAY = overlay

obj-m += $(LKM_NAME).o

all:	lkm	dt

dt:	$(DEV_TREE_OVERLAY).dts
	dtc -@ -I dts -O dtb -o $(DEV_TREE_OVERLAY).dtbo $(DEV_TREE_OVERLAY).dts

lkm:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf $(DEV_TREE_OVERLAY).dtbo	

load:
	sudo dtoverlay $(DEV_TREE_OVERLAY).dtbo
	sudo insmod $(LKM_NAME).ko

unload:
	sudo dtoverlay -r $(DEV_TREE_OVERLAY)
	sudo rmmod $(LKM_NAME)
