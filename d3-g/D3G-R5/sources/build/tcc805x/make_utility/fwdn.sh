    #!/bin/bash

echo "Start the FWDN V8"

main_path="deploy-images/tcc8050-main"
sub_path="deploy-images/tcc8050-sub"

func_checkUsb()
{
    DEVICE_ID="140e:b201 Telechips, Inc."
    USB_LIST=$(lsusb)

    if ! echo "$USB_LIST" | grep -q "$DEVICE_ID"; then
		echo "Not found device of $DEVICE_ID"
        exit 1
    fi 
}

func_preload()
{
    if [ -d "deploy-images/boot-firmware" ]; then
		echo -e "\nConnect FWDN V8 to Board"
		sudo ./fwdn --fwdn deploy-images/boot-firmware/fwdn.json

		if [ "$CORE" = "all" ]; then
			echo -e "\nLow-Format eMMC"
			sudo ./fwdn --storage emmc --low-format
		
            echo -e "\nDownload Pre-built F/W Image"
            sudo ./fwdn -w deploy-images/boot-firmware/boot.json
		fi
	else
		echo -e "\nNot exist boot-fimware file"
        exit 1
	fi
}

while true; do
    func_checkUsb

    echo -e "\nEnter the part you want to install: all, main, sub, mcu, QUIT"
    read -p "CORE: " CORE

    if [ "$CORE" = "all" ]; then
        if [ ! -d "deploy-images" ]; then
            echo -e "\nNot exist deploy-images directory"
            exit 1
		fi

		func_preload

        if [ -f "deploy-images/cr5_snor.rom" ]; then
			echo -e "\nMCU install"
            sudo ./fwdn -w deploy-images/cr5_snor.rom --area die1 --storage snor
        else
            echo -e "\nNot exist cr5_snor.rom file"
            exit 1
        fi

        echo -e "\nSD_Data install"
        if [ -f "deploy-images/SD_Data.fai" ]; then
            echo Download FAI File - SD_Data.fai
            sudo ./fwdn -w deploy-images/SD_Data.fai --storage emmc --area user
        fi
		
    elif [ "$CORE" = "main" ]; then
        echo -e "\nEnter the part you want to install on main-core: all, bootloader, kernel, filesystem, dtb, home"
        read -p "PART: " PART

        if [ ! -d $main_path ]; then
            echo -e "\nNot exist \"$main_path\" directory"
            exit 1
		fi
		
		func_preload

        if [ "$PART" = "all" ]; then
            echo -e "\nInstall the bootloader for main core\n"
            sudo ./fwdn -w $main_path/ca72_bl3.rom --storage emmc --area user --part bl3_ca72_a
            echo -e "\nInstall the kernel for main core\n"
            sudo ./fwdn -w $main_path/tc-boot-tcc8050-main.img --storage emmc --area user --part boot
            echo -e "\nInstall the filesystem for main core\n"
            sudo ./fwdn -w $main_path/automotive-linux-platform-image-tcc8050-main.ext4 --storage emmc --area user --part system
            echo -e "\nInstall the dtb for main core\n"
            sudo ./fwdn -w $main_path/tcc8050-linux-ivi-tost_sv0.1.dtb --storage emmc --area user --part dtb
            echo -e "\nInstall the home for main core\n"
            sudo ./fwdn -w deploy-images/home-directory.ext4 --storage emmc --area user --part home			
        elif [ "$PART" = "bootloader" ]; then
            echo -e "\nInstall the $PART for main core\n"
            sudo ./fwdn -w $main_path/ca72_bl3.rom --storage emmc --area user --part bl3_ca72_a
        elif [ "$PART" = "kernel" ]; then
            echo -e "\nInstall the $PART for main core\n"
            sudo ./fwdn -w $main_path/tc-boot-tcc8050-main.img --storage emmc --area user --part boot
        elif [ "$PART" = "filesystem" ]; then
            echo -e "\nInstall the $PART for main core\n"
            sudo ./fwdn -w $main_path/automotive-linux-platform-image-tcc8050-main.ext4 --storage emmc --area user --part system
        elif [ "$PART" = "dtb" ]; then
            echo -e "\nInstall the $PART for main core\n"
            sudo ./fwdn -w $main_path/tcc8050-linux-ivi-tost_sv0.1.dtb --storage emmc --area user --part dtb
        elif [ "$PART" = "home" ]; then
            echo -e "\nInstall the $PART for main core\n"
            sudo ./fwdn -w deploy-images/home-directory.ext4 --storage emmc --area user --part home
        else
            echo -e "\nInvalid input"
            exit 1
        fi
    elif [ "$CORE" = "sub" ]; then
        echo -e "\nEnter the part you want to install on sub-core: all, bootloader, kernel, filesystem, dtb, home"
        read -p "PART: " PART

        if [ ! -d $sub_path ]; then
            echo -e "\nNot exist $sub_path directory"
            exit 1
		fi
		
		func_preload

        if [ "$PART" = "all" ]; then
            echo -e "\nInstall the bootloader for sub core\n"
            sudo ./fwdn -w $sub_path/ca53_bl3.rom --storage emmc --area user --part bl3_ca53_a
            echo -e "\nInstall the kernel for sub core\n"
            sudo ./fwdn -w $sub_path/tc-boot-tcc8050-sub.img --storage emmc --area user --part subcore_boot
            echo -e "\nInstall the filesystem for sub core"
            sudo ./fwdn -w $sub_path/telechips-subcore-image-tcc8050-sub.ext4 --storage emmc --area user --part subcore_root
            echo -e "\nInstall the dtb for sub core"
            sudo ./fwdn -w $sub_path/tcc8050-linux-subcore-ivi-tost_sv0.1.dtb --storage emmc --area user --part subcore_dtb
            echo -e "\nInstall the home for sub core"
            sudo ./fwdn -w deploy-images/home-directory.ext4 --storage emmc --area user --part home
        elif [ "$PART" = "bootloader" ]; then
            echo -e "\nInstall the $PART for sub core\n"
            sudo ./fwdn -w $sub_path/ca53_bl3.rom --storage emmc --area user --part bl3_ca53_a
        elif [ "$PART" = "kernel" ]; then
            echo -e "\nInstall the $PART for sub core\n"
            sudo ./fwdn -w $sub_path/tc-boot-tcc8050-sub.img --storage emmc --area user --part subcore_boot
        elif [ "$PART" = "filesystem" ]; then
            echo -e "\nInstall the $PART for sub core\n"
            sudo ./fwdn -w $sub_path/telechips-subcore-image-tcc8050-sub.ext4 --storage emmc --area user --part subcore_root
        elif [ "$PART" = "dtb" ]; then
            echo -e "\nInstall the $PART for sub core\n"
            sudo ./fwdn -w $sub_path/tcc8050-linux-subcore-ivi-tost_sv0.1.dtb --storage emmc --area user --part subcore_dtb
        elif [ "$PART" = "home" ]; then
            echo -e "\nInstall the $PART for sub core\n"
            if [ -f "deploy-images/home-directory.ext4" ]; then
                sudo ./fwdn -w deploy-images/home-directory.ext4 --storage emmc --area user --part home			
            else
                echo -e "\nNot exist home-directory.ext4"
            fi
        else
            echo -e "\nInvalid input"
            exit 1
        fi
    elif [ "$CORE" = "mcu" ]; then
        if [ -f "deploy-images/cr5_snor.rom" ]; then
			echo -e "\nInstall MCU Image"
            sudo ./fwdn -w deploy-images/cr5_snor.rom --area die1 --storage snor		
		fi
    elif [ "$CORE" = "QUIT" ]; then
            exit 1
	fi
done