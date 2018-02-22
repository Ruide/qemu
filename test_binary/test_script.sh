#../build/arm-softmmu/qemu-system-arm -monitor stdio -S -M sensortag -kernel /home/rd/Desktop/qemu/test_binary/uartecho_from_hex.bin -D /home/rd/Desktop/qemu/test_binary/qemu_debug.log -d unimp,in_asm,guest_errors -device loader,file=/home/rd/Desktop/qemu/test_binary/rom,addr=0x10000000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG1,addr=0x50001000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG2,addr=0x50002000 -device loader,file=/home/rd/Desktop/qemu/test_binary/CCFG,addr=0x50003000 -serial tcp::4444,server &
if pgrep qemu-system; then
  pgrep qemu-system | xargs kill
fi
echo 'gdbserver' | ../build/arm-softmmu/qemu-system-arm -nographic -S -M sensortag -kernel /home/rd/Desktop/qemu/test_binary/uartecho_from_hex.bin -D /home/rd/Desktop/qemu/test_binary/qemu_debug.log -d unimp,in_asm,guest_errors`#,int,exec,cpu` -device loader,file=/home/rd/Desktop/qemu/test_binary/rom,addr=0x10000000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG1,addr=0x50001000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG2,addr=0x50002000 -device loader,file=/home/rd/Desktop/qemu/test_binary/CCFG,addr=0x50003000 -serial tcp::4444,server &
sleep .5
gnome-terminal -x telnet localhost 4444
gnome-terminal -x /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gdb -x /home/rd/Desktop/qemu/gdb_arm_init.txt
#../build/arm-softmmu/qemu-system-arm -monitor stdio -S -M sensortag -kernel /home/rd/Desktop/qemu/test_binary/uartecho_from_hex.bin -device loader,file=/home/rd/Desktop/qemu/test_binary/rom,addr=0x10000000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG1,addr=0x50001000 -device loader,file=/home/rd/Desktop/qemu/test_binary/FCFG2,addr=0x50002000 -device loader,file=/home/rd/Desktop/qemu/test_binary/CCFG,addr=0x50003000 -serial tcp::4444,server

#-monitor stdio

#-trace events=/home/rd/Desktop/qemu/test_binary/events_list

