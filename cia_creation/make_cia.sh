#!/bin/sh

bannertool makebanner -i banner.png -a banner.wav -o banner.bnr
bannertool makesmdh -s "CHIP-8" -l "CHIP-8 Emulator" -p "505e06b2" -i icon.png  -o icon.icn

makerom -f cia -o chip8.cia -DAPP_ENCRYPTED=false -rsf manifest.rsf -target t -exefslogo -elf chip8.elf -icon icon.icn -banner banner.bnr

