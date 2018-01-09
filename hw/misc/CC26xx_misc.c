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




//FLASH section



typedef struct SensortagFLASH_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section transfered from CC26xx.c
} SensortagFLASH_state;

static uint64_t SensortagFLASH_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
	uint64_t r;

    switch(offset){
 	case 0xA0:
    	r = 0xFFFFFF01;
       	return r;
    case 0x318:
    	r = 0x8B99A02F;
       	return r;
    case 0x31C:
    	r = 0x00000025;
    	return r;
	default:
		qemu_log_mask(LOG_UNIMP,
                      "SensortagFLASH_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagFLASH_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
	qemu_log_mask(LOG_UNIMP,
                      "SensortagFLASH_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
}

static const MemoryRegionOps SensortagFLASH_ops = {
    .read = SensortagFLASH_read, 
    .write = SensortagFLASH_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagFLASH "SensortagFLASH"

#define SensortagFLASH(obj) \
    OBJECT_CHECK(SensortagFLASH_state, (obj), Type_SensortagFLASH)



static void SensortagFLASH_realize(DeviceState *dev, Error **errp)
{
    SensortagFLASH_state *s = SensortagFLASH(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagFLASH_ops, s,
                          "FLASH", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
}

static Property SensortagFLASH_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagFLASH_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagFLASH_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagFLASH_realize;
    dc->props = SensortagFLASH_properties;
}


static const TypeInfo SensortagFLASH_info = {
    .name          = Type_SensortagFLASH,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagFLASH_state),
    .class_init    = SensortagFLASH_class_init, 
};

static void SensortagFLASH_register_types(void)
{
    type_register_static(&SensortagFLASH_info);
}

type_init(SensortagFLASH_register_types)





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
                      "SensortagFCFG_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
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



/*
//Empty ram section


typedef struct Empty_ram_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section transfered from CC26xx.c
} Empty_ram_state;

static uint64_t Empty_ram_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    

    qemu_log_mask(LOG_UNIMP,
                      "Empty_ram_read_unimplemented: Bad offset %x\n", (int)offset);
    return 0x0;
}

static void Empty_ram_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    qemu_log_mask(LOG_UNIMP,
                      "Empty_ram_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
}

static const MemoryRegionOps Empty_ram_ops = {
    .read = Empty_ram_read, 
    .write = Empty_ram_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


#define Empty_ram(obj) \
    OBJECT_CHECK(Empty_ram_state, (obj), "Empty_ram")



static void Empty_ram_realize(DeviceState *dev, Error **errp)
{
    Empty_ram_state *s = Empty_ram(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &Empty_ram_ops, s,
                          "Empty_ram", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
}

static Property Empty_ram_properties[] = {
    DEFINE_PROP_UINT64("size", Empty_ram_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void Empty_ram_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = Empty_ram_realize;
    dc->props = Empty_ram_properties;
}


static const TypeInfo Empty_ram_info = {
    .name          = "Empty_ram",
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(Empty_ram_state),
    .class_init    = Empty_ram_class_init, 
};

static void Empty_ram_register_types(void)
{
    type_register_static(&Empty_ram_info);
}

type_init(Empty_ram_register_types)


*/
