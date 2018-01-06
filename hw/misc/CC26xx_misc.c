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
#include "qapi/error.h"


typedef struct SensortagFCFG_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section transfered from CC26xx.c
} SensortagFCFG_state;


static uint64_t SensortagFCFG_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
	uint64_t r;

    //SensortagFCFG_state *s = (SensortagFCFG_state *)opaque;
    switch(offset){
 	case 0xA0:
    	r = 0xFFFFFF01;
       	return r;
    case 0x318:
    	r = 0x8B99A02F;
       	return r;
    //case 0x31A:
    //	r = 0x102F;
    //	return r;
    case 0x31C:
    	r = 0x00000025;
    	return r;
    	// gdb read 0x31C and 0x31E seperately, and when call 0x31C it return 0x2 size uint,
    	// and 0x00000025 shrink to -> 0x0025, when call read 0x31E, nothing happen, bc no 
    	// callback funtion set up here. we could add case 0x31e if needed.
    	// ldr return 0x04 size uint. when call read 0x31C.
//    case 0x31E:
//    	r = 0x0025;

    	return r;
	default:
		qemu_log_mask(LOG_UNIMP,
                      "SensortagFCFG_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagFCFG_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{    
	qemu_log_mask(LOG_UNIMP,
                      "SensortagFCFG_unimplemented: Bad offset %x\n Write value %x\n", (int)offset,(int)value);
}



static const MemoryRegionOps SensortagFCFG_ops = {
// .read callback is invoked whenever vCPU performs read op from guest addr space mapping of QEMU MMIO memory region
    .read = SensortagFCFG_read, 
// .write method gets executed upon vCPU writes to the region
    .write = SensortagFCFG_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagFCFG "SensortagFCFG"

#define SensortagFCFG(obj) \
    OBJECT_CHECK(SensortagFCFG_state, (obj), Type_SensortagFCFG)



static void SensortagFCFG_realize(DeviceState *dev, Error **errp)
{
    SensortagFCFG_state *s = SensortagFCFG(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagFCFG_ops, s,
                          "FCFG", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
}

static Property SensortagFCFG_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagFCFG_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};

/* static void SensortagFCFG_init(Object *obj)
{
    //DeviceState *dev = DEVICE(obj);
    SensortagFCFG_state *s = SensortagFCFG(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    //int n;

    memory_region_init_io(&s->iomem, obj, &SensortagFCFG_ops, s,
                          "FCFG", 0x2000);
    sysbus_init_mmio(sbd, &s->iomem);
}
*/


static void SensortagFCFG_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagFCFG_realize;
    dc->props = SensortagFCFG_properties;
}


static const TypeInfo SensortagFCFG_info = {
    .name          = Type_SensortagFCFG,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagFCFG_state),
    //.instance_init = SensortagFCFG_init,
    .class_init    = SensortagFCFG_class_init, 
    // class init and instance init does not to be exsist at same time, overlap functions in realize.
};

static void SensortagFCFG_register_types(void)
{
    type_register_static(&SensortagFCFG_info);
}

type_init(SensortagFCFG_register_types)

