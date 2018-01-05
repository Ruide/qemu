/*
 * TI sensortag misc
 *
 * Copyright (c) 2006 CodeSourcery.
 * Written by Paul Brook
 *
 * This code is licensed under the GPL.
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/log.h"


typedef struct SensortagFCFG_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
} SensortagFCFG_state;


static uint64_t SensortagFCFG_read(void *opaque, hwaddr offset,
                                   unsigned size)
{
    //SensortagFCFG_state *s = (SensortagFCFG_state *)opaque;
    switch(offset){
    case 0x318:
       	return 0x2BB4;
    case 0x31A:
    	return 0x102F;
    case 0x31C:
    	return 0x0;
    case 0x31E:
    	return 0x27;
	default:
		qemu_log_mask(LOG_UNIMP,
                      "SensortagFCFG_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}


static const MemoryRegionOps SensortagFCFG_ops = {
// .read callback is invoked whenever vCPU performs read op from guest addr space mapping of QEMU MMIO memory region
    .read = SensortagFCFG_read, 
// .write method gets executed upon vCPU writes to the region
//    .write = suart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagFCFG "SensortagFCFG"

#define SensortagFCFG(obj) \
    OBJECT_CHECK(SensortagFCFG_state, (obj), Type_SensortagFCFG)

static void SensortagFCFG_init(Object *obj)
{
    //DeviceState *dev = DEVICE(obj);
    SensortagFCFG_state *s = SensortagFCFG(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    //int n;

    memory_region_init_io(&s->iomem, obj, &SensortagFCFG_ops, s,
                          "FCFG", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);
}




static const TypeInfo SensortagFCFG_info = {
    .name          = Type_SensortagFCFG,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagFCFG_state),
    .instance_init = SensortagFCFG_init,
    //.class_init    = SensortagFCFG_class_init,
};

static void SensortagFCFG_register_types(void)
{
    type_register_static(&SensortagFCFG_info);
}

type_init(SensortagFCFG_register_types)

