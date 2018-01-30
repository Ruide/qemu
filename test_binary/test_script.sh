../build/arm-softmmu/qemu-system-arm -monitor stdio -S -M sensortag -kernel /home/rd/Desktop/qemu/test_binary/uartecho_from_hex.bin -D /home/rd/Desktop/qemu/test_binary/qemu_debug.log -d unimp,in_asm,guest_errors -device loader,file=/home/rd/Desktop/qemu/test_binary/rom,addr=0x10000000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG1,addr=0x50001000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG2,addr=0x50002000 -device loader,file=/home/rd/Desktop/qemu/test_binary/CCFG,addr=0x50003000 -serial tcp::4444,server
#../build/arm-softmmu/qemu-system-arm -monitor stdio -S -M sensortag -kernel /home/rd/Desktop/qemu/test_binary/uartecho_from_hex.bin -device loader,file=/home/rd/Desktop/qemu/test_binary/rom,addr=0x10000000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG1,addr=0x50001000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG2,addr=0x50002000 -device loader,file=/home/rd/Desktop/qemu/test_binary/CCFG,addr=0x50003000 -serial tcp::4444,server

#-monitor stdio

#-trace events=/home/rd/Desktop/qemu/test_binary/events_list

