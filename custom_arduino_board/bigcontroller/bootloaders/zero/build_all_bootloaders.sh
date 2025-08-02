#!/bin/bash -ex

BOARD_ID=bigcontroller NAME=samd21_sam_ba_bigcontroller make clean all
mv -v samd21_sam_ba_bigcontroller.* ../bigcontroller/

echo Done building bootloaders!
