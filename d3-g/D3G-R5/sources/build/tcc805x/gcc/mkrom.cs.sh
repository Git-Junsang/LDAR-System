#!/bin/bash
###################################################################################################
#
#   FileName : mkrom.cs.sh
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################
#
#   TCC Version 1.0
#
#   This source code contains confidential information of Telechips.
#
#   Any unauthorized use without a written permission of Telechips including not limited to
#   re-distribution in source or binary form is strictly prohibited.
#
#   This source code is provided "AS IS" and nothing contained in this source code shall constitute
#   any express or implied warranty of any kind, including without limitation, any warranty of
#   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
#   or other third party intellectual property right. No warranty is made, express or implied,
#   regarding the information's accuracy,completeness, or performance.
#
#   In no event shall Telechips be liable for any claim, damages or other liability arising from,
#   out of or in connection with this source code or the use in the source code.
#
#   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
#   Telechips and Company.
#   This source code is provided "AS IS" and nothing contained in this source code shall constitute
#   any express or implied warranty of any kind, including without limitation, any warranty
#   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
#   copyright or other third party intellectual property right. No warranty is made, express or
#   implied, regarding the information's accuracy, completeness, or performance.
#   In no event shall Telechips be liable for any claim, damages or other liability arising from,
#   out of or in connection with this source code or the use in the source code.
#   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
#   between Telechips and Company.
#
###################################################################################################

for ARGUMENT in "$@"
do
    KEY=$(echo $ARGUMENT | cut -f1 -d=)
    VALUE=$(echo $ARGUMENT | cut -f2 -d=)

    case "$KEY" in
            BOARD_NAME)    BOARD_NAME=${VALUE} ;;
            OUTPUT_PATH)   OUTPUT_PATH=${VALUE} ;;
	    UTILITY_DIR)   UTILITY_DIR=${VALUE} ;;    
            *)
    esac
done



SNOR_SIZE=4
UTILITY_DIR=../make_utility/tcc805x_snor_mkimage
OUTPUT_DIR=..
OUTPUT_FILE=tcc805x_snor_boot.rom

#source ./config.mk

pushd $UTILITY_DIR

#temporary use previous 805x tool
chmod 755 ./tcc805x-snor-mkimage

case $BOARD_NAME in
	TCC8050_EVM)
		echo "BOARD_NAME!! $BOARD_NAME"
		OUTPUT_FILE=tcc8050_snor.cs.rom
		./tcc805x-snor-mkimage -i ./tcc8050.cs.cfg -o $OUTPUT_DIR/$OUTPUT_FILE
		;;
	TCC8059_EVM)
		echo "BOARD_NAME!! $BOARD_NAME"
		OUTPUT_FILE=tcc8059_snor.cs.rom
		./tcc805x-snor-mkimage -i ./tcc8059.cs.cfg -o $OUTPUT_DIR/$OUTPUT_FILE
		;;
	*)
		echo "WRONG BOARD_NAME!! $BOARD_NAME"
		echo "Define BOARD_NAME by TCC8050 or TCC8059 in config.mk"
esac
popd

