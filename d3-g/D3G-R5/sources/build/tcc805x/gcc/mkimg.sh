###################################################################################################
#
#   FileName : mk_tc_img.sh
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

#/bin/sh
for ARGUMENT in "$@"
do
    KEY=$(echo $ARGUMENT | cut -f1 -d=)
    VALUE=$(echo $ARGUMENT | cut -f2 -d=)

    case "$KEY" in
        TOOL_PATH)      TOOL_PATH=${VALUE} ;;
        INPUT_PATH)     INPUT_PATH=${VALUE} ;;
        OUTPUT_PATH)    OUTPUT_PATH=${VALUE} ;;
        IMAGE_VERSION)  IMAGE_VERSION=${VALUE} ;;
        TARGET_ADDRESS) TARGET_ADDRESS=${VALUE} ;;
        *)
    esac
done

MKTOOL_INPUT=$INPUT_PATH/boot.bin
MKTOOL_OUTPUT=$OUTPUT_PATH/r5_fw.rom
MKTOOL_NAME=R5-FW

chmod 755 $TOOL_PATH/tcmktool
$TOOL_PATH/tcmktool $MKTOOL_INPUT $MKTOOL_OUTPUT $MKTOOL_NAME $IMAGE_VERSION $TARGET_ADDRESS

