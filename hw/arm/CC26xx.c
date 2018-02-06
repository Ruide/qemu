/*
 * TI sensortag cc2650
 *
 * Copyright (C) 2010 Andre Beckus
 *
 * Implementation based on
 * Olimex "cc2650 Users Manual Rev. H"
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "hw/arm/arm.h"
#include "hw/devices.h"
#include "qemu/timer.h"
#include "hw/i2c/i2c.h"
#include "net/net.h"
#include "hw/boards.h"
#include "qemu/log.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "hw/char/sensortag_uart.h"
#include "hw/timer/sensortag_aon_rtc.h"
#include "hw/misc/CC26xx_misc.h"
#include "hw/misc/unimp.h"
#include "cpu.h"


#define BP_OLED_I2C  0x01
#define NUM_IRQ_LINES 64


typedef const struct {
    const char *name;
    uint32_t did0;
    uint32_t did1;
    uint32_t dc0;
    uint32_t dc1;
    uint32_t dc2;
    uint32_t dc3;
    uint32_t dc4;
    uint32_t peripherals;
} CC26xx_board_info;


/* Board init.  */
static CC26xx_board_info CC26xx_boards[] = {
  { "sensortag",
    0,
    0x0032000e,
    0x001f001f, /* dc0 sram size & flash size, 0x1f => 2kb, we want 128kb flash, 20kb sram */
    0x001132bf,
    0x01071013,
    0x3f0f01ff,
    0x0000001f,
    BP_OLED_I2C // peripherals
  }
};

/*

   MemoryRegionOps

 * Memory region callbacks
 */


static void do_sys_reset(void *opaque, int n, int level)
{
    if (level) {
        qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
    }
}


static void CC26xx_init(MachineState *ms, CC26xx_board_info *board)
{
/*

Q: What does the error message "Because this program contains initialised RAM data it
may run successfully under Code Composer studio but not as a standalone system. If 
your flash program requires initialised data in RAM, you will need to write Flash
code to initialise Ram memory." mean and why does it occur?

The error you have received is a result of the fact that your whole application is
linked to RAM and not to flash memory. You need to check your linker command file
to see if you really link sections to the flash memory.

To be more elaborate, this appears each time you program the flash memory, if your 
application has code in RAM, to remind you that having application code in RAM prevents
your application from working as a standalone system. So you need modify your CMD file.

Please refer to the application note SPRA958H (includes also code examples) to get an
idea how to run an application from the DSP internal flash memory.


*/


    /*Table 4-2. Interrupts*/
/*
/Vector Number/ /Interrupt Number(Bit in Interrupt Registers)/ /Vector Address or Offset/ /Description/
0 to 15 – 0x0000 0000 to 0x0000 003C Processor exceptions
16 0 0x0000 0040 GPIO edge detect
17 1 0x0000 0044 I2C
18 2 0x0000 0048 RF Core and packet engine 1
19 3 0x0000 004C Unassigned
20 4 0x0000 0050 AON RTC
21 5 0x0000 0054 UART0
22 6 0x0000 0058 UART1
23 7 0x0000 005C SSI0
24 8 0x0000 0060 SSI1
25 9 0x0000 0064 RF Core and packet engine 2
26 10 0x0000 0068 RF Core hardware
27 11 0x0000 006C RF command acknowledge
28 12 0x0000 0070 I2S
29 13 0x0000 0074 Unassigned
30 14 0x0000 0078 Watchdog timer
31 15 0x0000 007C GPTimer 0A
32 16 0x0000 0080 GPTimer 0B
33 17 0x0000 0084 GPTimer 1A
34 18 0x0000 0088 GPTimer 1B
35 19 0x0000 008C GPTimer 2A
36 20 0x0000 0090 GPTimer 2B
37 21 0x0000 0094 GPTimer 3A
38 22 0x0000 0098 GPTimer 3B
39 23 0x0000 009C Crypto
40 24 0x0000 00A0 µDMA software defined
41 25 0x0000 00A4 µDMA error
42 26 0x0000 00A8 Flash
43 27 0x0000 00AC Software event 0www.ti.com Exception Model
44 28 0x0000 00B0 AUX combined event
45 29 0x0000 00B4 AON programmable event
46 30 0x0000 00B8 Dynamic programmable event
47 31 0x0000 00BC AUX comparator A
48 32 0x0000 00C0 AUX ADC new sample available or ADC DMA done, ADC underflow and overflow
49 33 0x0000 00C4 True random number generator
*/
    
/* unused function
    static const int uart_irq[] = {5, 6};
    static const int timer_irq[] = {15, 16, 17, 18, 19, 20, 21, 22};
    static const int gpio_irq[1] = {0};

    static const uint32_t gpio_addr[] = { 0x40022000};
*/

    int sram_size;
    int flash_size;

/*g_new()
#define g_new(struct_type, n_structs)

Allocates n_structs elements of type struct_type . The returned pointer is cast to a pointer to the
 given type. If n_structs is 0 it returns NULL. Care is taken to avoid overflow when calculating the
  size of the allocated block.
Since the returned pointer is already casted to the right type, it is normally unnecessary to cast
 it explicitly, and doing so might hide memory allocation errors.

Parameters:
struct_type: the type of the elements to allocate
n_structs: the number of elements to allocate

Returns
a pointer to the allocated memory, cast to a pointer to struct_type


*/

    MemoryRegion *sram = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    MemoryRegion *flash = g_new(MemoryRegion, 1);

/* Get the root memory region.  This interface should only be used temporarily
 * until a proper bus interface is available.
 */
    MemoryRegion *system_memory = get_system_memory();

    // flash_size = 128 * 1024; // 0x1f + 1 = 0x20, 0x20 << 1 = 0x40, 0x40 * 0x400 = 0x2000
    // sram_size = ((board->dc0 >> 18) + 1) * 1024;// 1024 <=> 0x400, 1fxxxx >> 18 = 7, 18 <=> 0x12, 
    //7 <=> 0b111, 0x1f <=> 0b11111
    // 0x(xxxx) <=> 16 bit 0b(x), 0x400 * 0x8 = 0x2000
    // LM3S6965 256kb flash 64kb sram

	flash_size = 0x00020000; // 0x1ffff -> 0x20000 128kb, 0x0003ffff 256kb, 
    // 20000 hex Bytes = 131072 decimal Bytes = 128KiB decimal = 128 * 1024 decimal = > * 8 bits
    sram_size = 0x00005000; // 0x4fff 20kb

/*

in this cc2650 sram: 2 sections

0x10000000 -> 0x1001cbfe (0x1cbfe)  117758 Bytes (~120KB) may come from debugger?? idk...
0x20000000 -> 0x20004ffe (0x4fff)    20478 Bytes (~20KB)

FLASH: 

0x0        -> 0x1ffff    (0x1ffff)  131071 Bytes (~128KB) 

No.  Memory             Address       Type      Access Permissions  Size
0    Flash              0x00000000    Normal    Full access, RO    512KB
1    SRAM               0x10000000    Normal    Full access, RW     32KB
2    SRAM               0x2007C000    Normal    Full access, RW     32KB
3    GPIO               0x2009C000    Device    Full access, RW     16KB
4    APB Peripherals    0x40000000    Device    Full access, RW    512KB
5    AHB Peripherals    0x50000000    Device    Full access, RW      2MB
6    PPB                0xE0000000    SO        Full access, RW      1MB

*/


/*
    GEL_MapAddStr(0x00000000, 0, 0x00020000, "R", 0);               * Flash 
    GEL_MapAddStr(0x10000000, 0, 0x00020000, "R", 0);               * ROM (internal rom)
    GEL_MapAddStr(0x11000000, 0, 0x00002000, "R|W", 0);             * GPRAM 
    GEL_MapAddStr(0x20000000, 0, 0x00005000, "R|W", 0);             * SRAM
    GEL_MapAddStr(0x21000000, 0, 0x00001000, "R|W", 0);             * RFC_SRAM
    ...

    see more details in cc26x0.gel

*/

/*
    GEL_MapAddStr(0x40000000, 0, 0x000E1028, "R|W", 0);             * Peripherals 
    GEL_MapAddStr(0x40031000, 0, 0x00001000, "NONE", 0);            * Protected 
    GEL_MapAddStr(0x40092000, 0, 0x00000030, "R|W|RUN_NONE", 0);    * AON_RTC, only read when halted
    GEL_MapAddStr(0x50001000, 0, 0x00000400, "R", 0);               * FCFG1 
    GEL_MapAddStr(0x50002000, 0, 0x00000400, "R", 0);               * FCFG2 
    GEL_MapAddStr(0x50003000, 0, 0x00001000, "R", 0);               * CCFG 
    GEL_MapAddStr(0xE0000000, 0, 0x00003000, "R|W", 0);             * CPU_ITM, CPU_DWT, CPU_FPB 
    GEL_MapAddStr(0xE000E000, 0, 0x00001000, "R|W", 0);             * CPU_SCS 
    GEL_MapAddStr(0xE0040000, 0, 0x00001000, "R|W", 0);             * CPU_TPIU 
    GEL_MapAddStr(0xE00FE000, 0, 0x00001000, "R|W", 0);             * CPU_TIPROP 


*/



    /* Flash programming is done via the SCU, so pretend it is ROM.  */
    memory_region_init_ram(flash, NULL, "CC26xx.flash", flash_size,
                           &error_fatal);
    memory_region_set_readonly(flash, true);
    memory_region_add_subregion(system_memory, 0, flash);

    memory_region_init_ram(sram, NULL, "CC26xx.sram", sram_size,
                           &error_fatal);
    memory_region_add_subregion(system_memory, 0x20000000, sram);


    int rom_size;
    MemoryRegion *rom = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    rom_size = 0x00020000; // same as flash, 0x20000 = 128KB
    //0x20000 => 131072Bytes => 65536Words
    memory_region_init_ram(rom, NULL, "CC26xx.rom", rom_size,
                           &error_fatal);
    memory_region_set_readonly(rom, true);
    memory_region_add_subregion(system_memory, 0x10000000, rom);

    int CCFG_size;
    MemoryRegion *CCFG = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    CCFG_size = 0x1000;
    memory_region_init_ram(CCFG, NULL, "CC26xx.CCFG", CCFG_size, &error_fatal);
    memory_region_set_readonly(CCFG, true);
    memory_region_add_subregion(system_memory, 0x50003000, CCFG);

    int FCFG1_size;
    MemoryRegion *FCFG1 = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    FCFG1_size = 0x400;
    memory_region_init_ram(FCFG1, NULL, "CC26xx.FCFG1", FCFG1_size, &error_fatal);
    memory_region_set_readonly(FCFG1, true);
    memory_region_add_subregion(system_memory, 0x50001000, FCFG1);

    int FCFG2_size;
    MemoryRegion *FCFG2 = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    FCFG2_size = 0x400;
    memory_region_init_ram(FCFG2, NULL, "CC26xx.FCFG2", FCFG2_size, &error_fatal);
    memory_region_set_readonly(FCFG2, true);
    memory_region_add_subregion(system_memory, 0x50002000, FCFG2);

    //0x400 => 1024Bytes => 256Words

    //int sram_size1;
    //MemoryRegion *sram1 = g_new(MemoryRegion, 1); // g_new(T, n) is neater than g_malloc(sizeof(T) * n).
    //sram_size1 = 0x0001cc00; // 0x1cc00 ~110KB
    //memory_region_init_ram(sram1, NULL, "CC26xx.sram1", sram_size1,
    //                       &error_fatal);
    //memory_region_add_subregion(system_memory, 0x10000000, sram1);


	DeviceState *nvic;
    nvic = armv7m_init(system_memory, flash_size, NUM_IRQ_LINES,
                       ms->kernel_filename, ms->cpu_type);

    qdev_connect_gpio_out_named(nvic, "SYSRESETREQ", 0,
                                qemu_allocate_irq(&do_sys_reset, NULL, 0));

    static const int uart_irq[] = {21};
    static const int rtc_irq[] = {20};


    suart_create(0x40001000,qdev_get_gpio_in(nvic, uart_irq[0]),serial_hds[0]);
    //create_unimplemented_device("FCFG",0x50001000, 0x2000);
//    SensortagFCFG_create(0x50001000, 0x2000);

    saon_rtc_create(0x40092000,qdev_get_gpio_in(nvic, rtc_irq[0]));
//    create_unimplemented_device("AON_RTC", 0x40092000, 0x1000);


/*steps adding dummy device:
1. in CC26xx.c    Dummydevice_create("SensortagAUX_WUC",0x400C6000, 0x1000);
2.1 in CC26xx_misc.c copy a section to top (e.g FLASH)
2.2 ctl + h, replace FLASH with AUX_WUC
2.3 replace member in SensortagFLASHState with new ones
2.4 modify offset in read and write function
*/

    //    create_unimplemented_device("FLASH", 0x40030000, 0x4000);
    Dummydevice_create("SensortagFLASH",0x40030000, 0x4000);
    //    create_empty_ram("Empty_ram",0x, 0x00010000);
    //    create_unimplemented_device("AUX_WUC", 0x400C6000, 0x1000); AUX Wake-up Controller
    Dummydevice_create("SensortagAUX_WUC",0x400C6000, 0x1000);// 0x43220294 to ?? need to test
    // 0x43220294 - 0x42000000 = 0x1220294, 0x1220294/0x20 = 0x91014 .... 0x14 //32bit to 1bit map. 
    //0x20 = 32.
    // 0x40091014 is AON_WUC_PWRSTAT = 0x0394707E
    //    create_unimplemented_device("PRCM", 0x40082000, 0x1000); Power, Clock, and Reset Management
    Dummydevice_create("SensortagPRCM",0x40082000, 0x1000);
    //    create_unimplemented_device("VIMS", 0x40034000, 0xC000); Versatile Instruction Memory System 
    //Control
    Dummydevice_create("SensortagVIMS",0x40034000, 0xC000);
    //    create_unimplemented_device("CCFG", 0x50003000, 0x1000);
//    Dummydevice_create("SensortagCCFG",0x50003000, 0x1000);
    //     create_unimplemented_device("AON_IOC", 0x40094000, 0x1000); Always-On Input/Output Controller
    Dummydevice_create("SensortagAON_IOC",0x40094000, 0x1000);
    //    create_unimplemented_device("AON_SYSCTL", 0x40090000, 0x1000); Always-On System Control
    Dummydevice_create("SensortagAON_SYSCTL",0x40090000, 0x1000);
    //    create_unimplemented_device("AON_WUC", 0x40091000, 0x1000); Always-On Wake-up Controller
    Dummydevice_create("SensortagAON_WUC",0x40091000, 0x1000);
    //    create_unimplemented_device("AUX_SMPH", 0x400C8000, 0x1000); AUX Semaphores
    Dummydevice_create("SensortagAUX_SMPH",0x400C8000, 0x1000);
    //    create_unimplemented_device("SMPH", 0x40084000, 0xC000); System CPU Semaphores
    Dummydevice_create("SensortagSMPH",0x40084000, 0xC000);
    //    create_unimplemented_device("AON_BATMON", 0x40095000, 0x2C000); Always-On Battery and Temperature 
    //Monitor 
    Dummydevice_create("SensortagAON_BATMON",0x40095000, 0x2C000);
    //    create_unimplemented_device("AUX_DDI0_OSC", 0x400CA000, 0x1000);
    Dummydevice_create("SensortagAUX_DDI0_OSC",0x400CA000, 0x1000);
    
    


    /* 2 bit-banding regions of memory (to avoid read-modify-write(which is effected by interrupt))
    1. addresses from 0x22000000 to 0x220FFFFF are used for bit-banding the 32KB region from 0x20000000 
    to 0x20007FFF
    2. addresses from 0x42000000 to 0x43FFFFFF are used for bit-banding the 1 MB region from 0x40000000 
    to 0x400FFFFF
    
    0x20000000 ==> Base address of SRAM
    0x22000000 ==> Base address of SRAM alias region
    (0x22000000 ~ 0x220FFFFF)

    0x40000000 ==> Base address of peripheral region
    0x42000000 ==> Base address of peripheral alias region
    (0x42000000 ~ 0x43FFFFFF)
    
    so, write to a 32bit value in memory address in bit-banding alias region is equal to write a 1bit
     value in bit-banding region.
    
    e.g. 0x42600484 mapped to 0x40030024
    */
    /*
    FCFG1 Register Summary
    0x500010A0 ~ 0x5000141C
    */


     /* Add dummy regions for the devices we don't implement yet,
     * so guest accesses don't cause unlogged crashes.
     */    

    create_unimplemented_device("RF-core", 0x21000000, 0x1E000000);
    create_unimplemented_device("SSI-0", 0x40000000, 0x1000);
    create_unimplemented_device("I2C-0", 0x40002000, 0x1000);
    create_unimplemented_device("GPT0", 0x40010000, 0x1000);
    create_unimplemented_device("GPT1", 0x40011000, 0x1000);
    create_unimplemented_device("GPT2", 0x40012000, 0x1000);
    create_unimplemented_device("GPT3", 0x40013000, 0x1000);
    create_unimplemented_device("UDMA-0", 0x40020000, 0x1000);
    create_unimplemented_device("I2S-0", 0x40021000, 0x1000);
    create_unimplemented_device("GPIO", 0x40022000, 0x2000);
    create_unimplemented_device("CRYPTO", 0x40024000, 0x4000);
    create_unimplemented_device("TRNG", 0x40028000, 0x8000);
    create_unimplemented_device("RF-core-PWR", 0x40040000, 0x1000);
    create_unimplemented_device("RF-core-DBELL", 0x40041000, 0x2000);
    create_unimplemented_device("RF-core-RAT", 0x40043000, 0x3D000);
    create_unimplemented_device("WDT", 0x40080000, 0x1000);
    create_unimplemented_device("IOC", 0x40081000, 0x1000);
    create_unimplemented_device("EVENT", 0x40083000, 0x1000);
    create_unimplemented_device("AON_EVENT", 0x40093000, 0x1000);
    create_unimplemented_device("AUX_AIODIO-0", 0x400C1000, 0x1000);
    create_unimplemented_device("AUX_AIODIO-1", 0x400C2000, 0x2000);
    create_unimplemented_device("AUX_TDCIF", 0x400C4000, 0x1000);
    create_unimplemented_device("AUX_EVCTL", 0x400C5000, 0x1000);
    create_unimplemented_device("AUX_TIMER", 0x400C7000, 0x1000);
    create_unimplemented_device("AUX_ANAIF", 0x400C9000, 0x1000);
    create_unimplemented_device("AUX_ADI4", 0x400CB000, 0x15000);
    create_unimplemented_device("AUX_RAM", 0x400E0000, 0x1000);
    create_unimplemented_device("AUX_SCE", 0x400E1000, 0xFF20000);
    create_unimplemented_device("CPU_ITM", 0xE0000000, 0x1000);
    create_unimplemented_device("CPU_DWT", 0xE0001000, 0x1000);
    create_unimplemented_device("CPU_FPB", 0xE0002000, 0xC000);
    create_unimplemented_device("CPU_SCS", 0xE000E000, 0x32000);
    create_unimplemented_device("CPU_TPIU", 0xE0040000, 0x1000);
}



/* FIXME: Figure out how to generate these from stellaris_boards.  */
static void Sensortag_init(MachineState *machine)
{
    
    CC26xx_init(machine, &CC26xx_boards[0]);
}


static void Sensortag_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "CC26xx sensortag";
    mc->init = Sensortag_init;
    mc->ignore_memory_transaction_failures = true;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-m3");
}


static const TypeInfo Sensortag_type = {
    .name = MACHINE_TYPE_NAME("sensortag"),
    .parent = TYPE_MACHINE,
    .class_init = Sensortag_class_init,
};


static void CC26xx_machine_init(void)
{
    type_register_static(&Sensortag_type);
}

type_init(CC26xx_machine_init)



/*Table 3-2. Memory Map
cortex M3 memory map

/Base Address/ /Module/ /Module Name/
0x0000 0000 FLASHMEM On-Chip Flash
0x2000 0000 SRAM Low-Leakage RAM
0x2100 0000 RFC_RAM RF Core RAM
0x4000 0000 SSI0 Synchronous Serial Interface 0
0x4000 1000 UART0 Universal Asynchronous Receiver/Transmitter 0
0x4000 2000 I2C0 I2C Master/Slave Serial Controller 0
0x4000 8000 SSI1 Synchronous Serial Interface 1
0x4001 0000 GPT0 General Purpose Timer 0
0x4001 1000 GPT1 General Purpose Timer 1
0x4001 2000 GPT2 General Purpose Timer 2
0x4001 3000 GPT3 General Purpose Timer 3
0x4002 0000 UDMA0 Micro Direct Memory Access Controller 0
0x4002 1000 I2S0 I2S Audio DMA 0
0x4002 2000 GPIO General Purpose Input/Output
0x4002 4000 CRYPTO Cryptography Engine
0x4002 8000 TRNG True Random Number Generator
0x4003 0000 FLASH Flash Controller
0x4003 4000 VIMS Versatile Instruction Memory System Control
0x4004 0000 RFC_PWR RF Core Power
0x4004 1000 RFC_DBELL RF Core Doorbell
0x4004 3000 RFC_RAT RF Core Radio Timer
0x4008 0000 WDT Watchdog Timer
0x4008 1000 IOC Input/Output Controller
0x4008 2000 PRCM Power, Clock, and Reset Management
0x4008 3000 EVENT Event Fabric
0x4008 4000 SMPH System CPU Semaphores
0x4009 0000 AON_SYSCTL Always-On System Control
0x4009 1000 AON_WUC Always-On Wake-up Controller
0x4009 2000 AON_RTC Always-On Real Time Clock
0x4009 3000 AON_EVENT Always-On Event
0x4009 4000 AON_IOC Always-On Input/Output Controller
0x4009 5000 AON_BATMON Always-On Battery and Temperature Monitor
0x400C 1000 AUX_AIODIO0 AUX Analog/Digital Input/Output Control 0
0x400C 2000 AUX_AIODIO1 AUX Analog/Digital Input/Output Control 1
0x400C 4000 AUX_TDCIF AUX Time-to Digital Converter Interface
0x400C 5000 AUX_EVCTL AUX Event Control
0x400C 6000 AUX_WUC AUX Wake-up Controller
0x400C 7000 AUX_TIMER AUX Timer
0x400C 8000 AUX_SMPH AUX Semaphores
0x400C 9000 AUX_ANAIF AUX Analog Interface
0x400C A000 AUX_DDI0_OSC AUX Digital/Digital Interface, Oscillator control
0x400C B000 AUX_ADI4 AUX Analog/Digital Interface 4
0x400E 0000 AUX_RAM AUX RAM
0x400E 1000 AUX_SCE AUX Sensor Control Engine
0x5000 1000 FCFG1 Factory Configuration Area 1
0x5000 3000 CCFG Customer Configuration AreaFunctional Description www.ti.com
0xE000 0000 CPU_ITM Cortex-M Instrumentation Trace Macrocell
0xE000 1000 CPU_DWT Cortex-M Data Watchpoint and Trace
0xE000 2000 CPU_FPB Cortex-M Flash Patch and Breakpoint
0xE000 E000 CPU_SCS Cortex-M System Control Space
0xE004 0000 CPU_TPIU Cortex-M Trace Port Interface Unit
*/

/* check /test_binaries/rtos_rom.map for more details.

MEMORY CONFIGURATION

         name            origin    length      used     unused   attr    fill
----------------------  --------  ---------  --------  --------  ----  --------
  VECS                  00000000   00000100  0000003c  000000c4  R  X
  RTOS_FLASH            00000100   00000700  000004e0  00000220  R  X
  FLASH                 00000800   0001f000  000014f0  0001db10  R  X
  FLASH_CCFG            0001ffa8   00000058  00000058  00000000  R  X
  RTOS_ROM              1001a000   00002c00  00002a92  0000016e  R  X
  VRAM                  20000000   00000100  000000dc  00000024  RW X
  RTOS_SRAM             20000100   00000500  00000104  000003fc  RW X
  SRAM                  20000600   00004a00  00001c20  00002de0  RW X



*/

/*
 * DeviceState:
 * @realized: Indicates whether the device has been fully constructed.
 *
 * This structure should not be accessed directly.  We declare it here
 * so that it can be embedded in individual device state structures.
 */

// https://github.com/qemu/qemu/blob/master/docs/qdev-device-use.txt


/*Table 4-2. Interrupts
cortex M3 irq

/Vector Number/ /Interrupt Number(Bit in Interrupt Registers)/ /Vector Address or Offset/ /Description/
0 to 15 – 0x0000 0000 to 0x0000 003C Processor exceptions
16 0 0x0000 0040 GPIO edge detect
17 1 0x0000 0044 I2C
18 2 0x0000 0048 RF Core and packet engine 1
19 3 0x0000 004C Unassigned
20 4 0x0000 0050 AON RTC
21 5 0x0000 0054 UART0
22 6 0x0000 0058 UART1
23 7 0x0000 005C SSI0
24 8 0x0000 0060 SSI1
25 9 0x0000 0064 RF Core and packet engine 2
26 10 0x0000 0068 RF Core hardware
27 11 0x0000 006C RF command acknowledge
28 12 0x0000 0070 I2S
29 13 0x0000 0074 Unassigned
30 14 0x0000 0078 Watchdog timer
31 15 0x0000 007C GPTimer 0A
32 16 0x0000 0080 GPTimer 0B
33 17 0x0000 0084 GPTimer 1A
34 18 0x0000 0088 GPTimer 1B
35 19 0x0000 008C GPTimer 2A
36 20 0x0000 0090 GPTimer 2B
37 21 0x0000 0094 GPTimer 3A
38 22 0x0000 0098 GPTimer 3B
39 23 0x0000 009C Crypto
40 24 0x0000 00A0 µDMA software defined
41 25 0x0000 00A4 µDMA error
42 26 0x0000 00A8 Flash
43 27 0x0000 00AC Software event 0www.ti.com Exception Model
44 28 0x0000 00B0 AUX combined event
45 29 0x0000 00B4 AON programmable event
46 30 0x0000 00B8 Dynamic programmable event
47 31 0x0000 00BC AUX comparator A
48 32 0x0000 00C0 AUX ADC new sample available or ADC DMA done, ADC underflow and overflow
49 33 0x0000 00C4 True random number generator

*/

/*
EVENT Registers
Table 4-14. EVENT Registers

0x4008 3000 EVENT Event Fabric

/Offset/ /Acronym Register Name/ /Section/
0h CPUIRQSEL0 Output Selection for CPU Interrupt 0 Section 4.7.2.1
4h CPUIRQSEL1 Output Selection for CPU Interrupt 1 Section 4.7.2.2
8h CPUIRQSEL2 Output Selection for CPU Interrupt 2 Section 4.7.2.3
Ch CPUIRQSEL3 Output Selection for CPU Interrupt 3 Section 4.7.2.4
10h CPUIRQSEL4 Output Selection for CPU Interrupt 4 Section 4.7.2.5
14h CPUIRQSEL5 Output Selection for CPU Interrupt 5 Section 4.7.2.6
18h CPUIRQSEL6 Output Selection for CPU Interrupt 6 Section 4.7.2.7
1Ch CPUIRQSEL7 Output Selection for CPU Interrupt 7 Section 4.7.2.8
20h CPUIRQSEL8 Output Selection for CPU Interrupt 8 Section 4.7.2.9
24h CPUIRQSEL9 Output Selection for CPU Interrupt 9 Section 4.7.2.10
28h CPUIRQSEL10 Output Selection for CPU Interrupt 10 Section 4.7.2.11
2Ch CPUIRQSEL11 Output Selection for CPU Interrupt 11 Section 4.7.2.12
30h CPUIRQSEL12 Output Selection for CPU Interrupt 12 Section 4.7.2.13
34h CPUIRQSEL13 Output Selection for CPU Interrupt 13 Section 4.7.2.14
38h CPUIRQSEL14 Output Selection for CPU Interrupt 14 Section 4.7.2.15
3Ch CPUIRQSEL15 Output Selection for CPU Interrupt 15 Section 4.7.2.16
40h CPUIRQSEL16 Output Selection for CPU Interrupt 16 Section 4.7.2.17
44h CPUIRQSEL17 Output Selection for CPU Interrupt 17 Section 4.7.2.18
48h CPUIRQSEL18 Output Selection for CPU Interrupt 18 Section 4.7.2.19
4Ch CPUIRQSEL19 Output Selection for CPU Interrupt 19 Section 4.7.2.20
50h CPUIRQSEL20 Output Selection for CPU Interrupt 20 Section 4.7.2.21
54h CPUIRQSEL21 Output Selection for CPU Interrupt 21 Section 4.7.2.22
58h CPUIRQSEL22 Output Selection for CPU Interrupt 22 Section 4.7.2.23
5Ch CPUIRQSEL23 Output Selection for CPU Interrupt 23 Section 4.7.2.24
60h CPUIRQSEL24 Output Selection for CPU Interrupt 24 Section 4.7.2.25
64h CPUIRQSEL25 Output Selection for CPU Interrupt 25 Section 4.7.2.26
68h CPUIRQSEL26 Output Selection for CPU Interrupt 26 Section 4.7.2.27
6Ch CPUIRQSEL27 Output Selection for CPU Interrupt 27 Section 4.7.2.28
70h CPUIRQSEL28 Output Selection for CPU Interrupt 28 Section 4.7.2.29
74h CPUIRQSEL29 Output Selection for CPU Interrupt 29 Section 4.7.2.30
78h CPUIRQSEL30 Output Selection for CPU Interrupt 30 Section 4.7.2.31
7Ch CPUIRQSEL31 Output Selection for CPU Interrupt 31 Section 4.7.2.32
80h CPUIRQSEL32 Output Selection for CPU Interrupt 32 Section 4.7.2.33
84h CPUIRQSEL33 Output Selection for CPU Interrupt 33 Section 4.7.2.34
100h RFCSEL0 Output Selection for RFC Event 0 Section 4.7.2.35
104h RFCSEL1 Output Selection for RFC Event 1 Section 4.7.2.36
108h RFCSEL2 Output Selection for RFC Event 2 Section 4.7.2.37
10Ch RFCSEL3 Output Selection for RFC Event 3 Section 4.7.2.38
110h RFCSEL4 Output Selection for RFC Event 4 Section 4.7.2.39
114h RFCSEL5 Output Selection for RFC Event 5 Section 4.7.2.40
118h RFCSEL6 Output Selection for RFC Event 6 Section 4.7.2.41
11Ch RFCSEL7 Output Selection for RFC Event 7 Section 4.7.2.42
120h RFCSEL8 Output Selection for RFC Event 8 Section 4.7.2.43
124h RFCSEL9 Output Selection for RFC Event 9 Section 4.7.2.44www.ti.com Interrupts and Events Registers
200h GPT0ACAPTSEL Output Selection for GPT0 0 Section 4.7.2.45
204h GPT0BCAPTSEL Output Selection for GPT0 1 Section 4.7.2.46
300h GPT1ACAPTSEL Output Selection for GPT1 0 Section 4.7.2.47
304h GPT1BCAPTSEL Output Selection for GPT1 1 Section 4.7.2.48
400h GPT2ACAPTSEL Output Selection for GPT2 0 Section 4.7.2.49
404h GPT2BCAPTSEL Output Selection for GPT2 1 Section 4.7.2.50
508h UDMACH1SSEL Output Selection for DMA Channel 1 SREQ Section 4.7.2.51
50Ch UDMACH1BSEL Output Selection for DMA Channel 1 REQ Section 4.7.2.52
510h UDMACH2SSEL Output Selection for DMA Channel 2 SREQ Section 4.7.2.53
514h UDMACH2BSEL Output Selection for DMA Channel 2 REQ Section 4.7.2.54
518h UDMACH3SSEL Output Selection for DMA Channel 3 SREQ Section 4.7.2.55
51Ch UDMACH3BSEL Output Selection for DMA Channel 3 REQ Section 4.7.2.56
520h UDMACH4SSEL Output Selection for DMA Channel 4 SREQ Section 4.7.2.57
524h UDMACH4BSEL Output Selection for DMA Channel 4 REQ Section 4.7.2.58
528h UDMACH5SSEL Output Selection for DMA Channel 5 SREQ Section 4.7.2.59
52Ch UDMACH5BSEL Output Selection for DMA Channel 5 REQ Section 4.7.2.60
530h UDMACH6SSEL Output Selection for DMA Channel 6 SREQ Section 4.7.2.61
534h UDMACH6BSEL Output Selection for DMA Channel 6 REQ Section 4.7.2.62
538h UDMACH7SSEL Output Selection for DMA Channel 7 SREQ Section 4.7.2.63
53Ch UDMACH7BSEL Output Selection for DMA Channel 7 REQ Section 4.7.2.64
540h UDMACH8SSEL Output Selection for DMA Channel 8 SREQ Section 4.7.2.65
544h UDMACH8BSEL Output Selection for DMA Channel 8 REQ Section 4.7.2.66
548h UDMACH9SSEL Output Selection for DMA Channel 9 SREQ Section 4.7.2.67
54Ch UDMACH9BSEL Output Selection for DMA Channel 9 REQ Section 4.7.2.68
550h UDMACH10SSEL Output Selection for DMA Channel 10 SREQ Section 4.7.2.69
554h UDMACH10BSEL Output Selection for DMA Channel 10 REQ Section 4.7.2.70
558h UDMACH11SSEL Output Selection for DMA Channel 11 SREQ Section 4.7.2.71
55Ch UDMACH11BSEL Output Selection for DMA Channel 11 REQ Section 4.7.2.72
560h UDMACH12SSEL Output Selection for DMA Channel 12 SREQ Section 4.7.2.73
564h UDMACH12BSEL Output Selection for DMA Channel 12 REQ Section 4.7.2.74
56Ch UDMACH13BSEL Output Selection for DMA Channel 13 REQ Section 4.7.2.75
574h UDMACH14BSEL Output Selection for DMA Channel 14 REQ Section 4.7.2.76
57Ch UDMACH15BSEL Output Selection for DMA Channel 15 REQ Section 4.7.2.77
580h UDMACH16SSEL Output Selection for DMA Channel 16 SREQ Section 4.7.2.78
584h UDMACH16BSEL Output Selection for DMA Channel 16 REQ Section 4.7.2.79
588h UDMACH17SSEL Output Selection for DMA Channel 17 SREQ Section 4.7.2.80
58Ch UDMACH17BSEL Output Selection for DMA Channel 17 REQ Section 4.7.2.81
5A8h UDMACH21SSEL Output Selection for DMA Channel 21 SREQ Section 4.7.2.82
5ACh UDMACH21BSEL Output Selection for DMA Channel 21 REQ Section 4.7.2.83
5B0h UDMACH22SSEL Output Selection for DMA Channel 22 SREQ Section 4.7.2.84
5B4h UDMACH22BSEL Output Selection for DMA Channel 22 REQ Section 4.7.2.85
5B8h UDMACH23SSEL Output Selection for DMA Channel 23 SREQ Section 4.7.2.86
5BCh UDMACH23BSEL Output Selection for DMA Channel 23 REQ Section 4.7.2.87
5C0h UDMACH24SSEL Output Selection for DMA Channel 24 SREQ Section 4.7.2.88
5C4h UDMACH24BSEL Output Selection for DMA Channel 24 REQ Section 4.7.2.89
600h GPT3ACAPTSEL Output Selection for GPT3 0 Section 4.7.2.90
604h GPT3BCAPTSEL Output Selection for GPT3 1 Section 4.7.2.91Interrupts and Events Registers www.ti.com
700h AUXSEL0 Output Selection for AUX Subscriber 0 Section 4.7.2.92
800h CM3NMISEL0 Output Selection for NMI Subscriber 0 Section 4.7.2.93
900h I2SSTMPSEL0 Output Selection for I2S Subscriber 0 Section 4.7.2.94
A00h FRZSEL0 Output Selection for FRZ Subscriber Section 4.7.2.95
F00h SWEV Set or Clear Software Events Section 4.7.2.96
*/

/* CPU_SCS Registers

Offset Acronym Register Name Section
4h ICTR Interrupt Control Type Section 2.7.4.1
8h ACTLR Auxiliary Control Section 2.7.4.2
10h STCSR SysTick Control and Status Section 2.7.4.3
14h STRVR SysTick Reload Value Section 2.7.4.4
18h STCVR SysTick Current Value Section 2.7.4.5
1Ch STCR SysTick Calibration Value Section 2.7.4.6
100h NVIC_ISER0 Irq 0 to 31 Set Enable Section 2.7.4.7
104h NVIC_ISER1 Irq 32 to 63 Set Enable Section 2.7.4.8
180h NVIC_ICER0 Irq 0 to 31 Clear Enable Section 2.7.4.9
184h NVIC_ICER1 Irq 32 to 63 Clear Enable Section 2.7.4.10
200h NVIC_ISPR0 Irq 0 to 31 Set Pending Section 2.7.4.11
204h NVIC_ISPR1 Irq 32 to 63 Set Pending Section 2.7.4.12
280h NVIC_ICPR0 Irq 0 to 31 Clear Pending Section 2.7.4.13
284h NVIC_ICPR1 Irq 32 to 63 Clear Pending Section 2.7.4.14
Offset Acronym Register Name Section
300h NVIC_IABR0 Irq 0 to 31 Active Bit Section 2.7.4.15
304h NVIC_IABR1 Irq 32 to 63 Active Bit Section 2.7.4.16
400h NVIC_IPR0 Irq 0 to 3 Priority Section 2.7.4.17
404h NVIC_IPR1 Irq 4 to 7 Priority Section 2.7.4.18
408h NVIC_IPR2 Irq 8 to 11 Priority Section 2.7.4.19
40Ch NVIC_IPR3 Irq 12 to 15 Priority Section 2.7.4.20
410h NVIC_IPR4 Irq 16 to 19 Priority Section 2.7.4.21
414h NVIC_IPR5 Irq 20 to 23 Priority Section 2.7.4.22
418h NVIC_IPR6 Irq 24 to 27 Priority Section 2.7.4.23
41Ch NVIC_IPR7 Irq 28 to 31 Priority Section 2.7.4.24
420h NVIC_IPR8 Irq 32 to 35 Priority Section 2.7.4.25
D00h CPUID CPUID Base Section 2.7.4.26
D04h ICSR Interrupt Control State Section 2.7.4.27
D08h VTOR Vector Table Offset Section 2.7.4.28
D0Ch AIRCR Application Interrupt/Reset Control Section 2.7.4.29
D10h SCR System Control Section 2.7.4.30
D14h CCR Configuration Control Section 2.7.4.31
D18h SHPR1 System Handlers 4-7 Priority Section 2.7.4.32
D1Ch SHPR2 System Handlers 8-11 Priority Section 2.7.4.33
D20h SHPR3 System Handlers 12-15 Priority Section 2.7.4.34
D24h SHCSR System Handler Control and State Section 2.7.4.35
D28h CFSR Configurable Fault Status Section 2.7.4.36
D2Ch HFSR Hard Fault Status Section 2.7.4.37
D30h DFSR Debug Fault Status Section 2.7.4.38
D34h MMFAR Mem Manage Fault Address Section 2.7.4.39
D38h BFAR Bus Fault Address Section 2.7.4.40
D3Ch AFSR Auxiliary Fault Status Section 2.7.4.41
D40h ID_PFR0 Processor Feature 0 Section 2.7.4.42
D44h ID_PFR1 Processor Feature 1 Section 2.7.4.43
D48h ID_DFR0 Debug Feature 0 Section 2.7.4.44
D4Ch ID_AFR0 Auxiliary Feature 0 Section 2.7.4.45
D50h ID_MMFR0 Memory Model Feature 0 Section 2.7.4.46
D54h ID_MMFR1 Memory Model Feature 1 Section 2.7.4.47
D58h ID_MMFR2 Memory Model Feature 2 Section 2.7.4.48
D5Ch ID_MMFR3 Memory Model Feature 3 Section 2.7.4.49
D60h ID_ISAR0 ISA Feature 0 Section 2.7.4.50
D64h ID_ISAR1 ISA Feature 1 Section 2.7.4.51
D68h ID_ISAR2 ISA Feature 2 Section 2.7.4.52
D6Ch ID_ISAR3 ISA Feature 3 Section 2.7.4.53
D70h ID_ISAR4 ISA Feature 4 Section 2.7.4.54
D88h CPACR Coprocessor Access Control Section 2.7.4.55
DF0h DHCSR Debug Halting Control and Status Section 2.7.4.56
DF4h DCRSR Deubg Core Register Selector Section 2.7.4.57
DF8h DCRDR Debug Core Register Data Section 2.7.4.58
DFCh DEMCR Debug Exception and Monitor Control Section 2.7.4.59
F00h STIR Software Trigger Interrupt Section 2.7.4.60
*/