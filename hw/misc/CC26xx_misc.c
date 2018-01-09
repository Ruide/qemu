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



/*steps adding dummy device:
1. in CC26xx.c    Dummydevice_create("SensortagAUX_WUC",0x400C6000, 0x1000);
2.1 in CC26xx_misc.c copy a section to top (e.g FLASH)
2.2 ctl + h, replace FLASH with AUX_WUC
2.3 replace member in SensortagFLASHState with new ones (e.g. CFG -> xxx)
2.4 modify offset in read and write function
2.5 modify initialization in _realize
*/




//AON_SYSCTL section 0x40090000



typedef struct SensortagAON_SYSCTL_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t SLEEPCTL;
} SensortagAON_SYSCTL_state;

static uint64_t SensortagAON_SYSCTL_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagAON_SYSCTL_state *s = (SensortagAON_SYSCTL_state *)opaque;

    switch(offset){
    case 0x8:
        return s->SLEEPCTL;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagAON_SYSCTL_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagAON_SYSCTL_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagAON_SYSCTL_state *s = (SensortagAON_SYSCTL_state *)opaque;
    switch(offset){
    case 0x8:
        s->SLEEPCTL = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagAON_SYSCTL_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagAON_SYSCTL_ops = {
    .read = SensortagAON_SYSCTL_read, 
    .write = SensortagAON_SYSCTL_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagAON_SYSCTL "SensortagAON_SYSCTL"

#define SensortagAON_SYSCTL(obj) \
    OBJECT_CHECK(SensortagAON_SYSCTL_state, (obj), Type_SensortagAON_SYSCTL)



static void SensortagAON_SYSCTL_realize(DeviceState *dev, Error **errp)
{
    SensortagAON_SYSCTL_state *s = SensortagAON_SYSCTL(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagAON_SYSCTL_ops, s,
                          "AON_SYSCTL", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->SLEEPCTL = 0x1;
}

static Property SensortagAON_SYSCTL_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagAON_SYSCTL_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagAON_SYSCTL_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagAON_SYSCTL_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagAON_SYSCTL_properties;

}


static const TypeInfo SensortagAON_SYSCTL_info = {
    .name          = Type_SensortagAON_SYSCTL,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagAON_SYSCTL_state),
    .class_init    = SensortagAON_SYSCTL_class_init, 
};

static void SensortagAON_SYSCTL_register_types(void)
{
    type_register_static(&SensortagAON_SYSCTL_info);
}

type_init(SensortagAON_SYSCTL_register_types)



//AON_IOC section 0x40094000


typedef struct SensortagAON_IOC_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t IOCLATCH;
} SensortagAON_IOC_state;

static uint64_t SensortagAON_IOC_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagAON_IOC_state *s = (SensortagAON_IOC_state *)opaque;

    switch(offset){
    case 0xc:
        return s->IOCLATCH;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagAON_IOC_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagAON_IOC_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagAON_IOC_state *s = (SensortagAON_IOC_state *)opaque;
    switch(offset){
    case 0xc:
        s->IOCLATCH = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagAON_IOC_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagAON_IOC_ops = {
    .read = SensortagAON_IOC_read, 
    .write = SensortagAON_IOC_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagAON_IOC "SensortagAON_IOC"

#define SensortagAON_IOC(obj) \
    OBJECT_CHECK(SensortagAON_IOC_state, (obj), Type_SensortagAON_IOC)



static void SensortagAON_IOC_realize(DeviceState *dev, Error **errp)
{
    SensortagAON_IOC_state *s = SensortagAON_IOC(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagAON_IOC_ops, s,
                          "AON_IOC", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->IOCLATCH = 0x1;
}

static Property SensortagAON_IOC_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagAON_IOC_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagAON_IOC_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagAON_IOC_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagAON_IOC_properties;

}


static const TypeInfo SensortagAON_IOC_info = {
    .name          = Type_SensortagAON_IOC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagAON_IOC_state),
    .class_init    = SensortagAON_IOC_class_init, 
};

static void SensortagAON_IOC_register_types(void)
{
    type_register_static(&SensortagAON_IOC_info);
}

type_init(SensortagAON_IOC_register_types)




//FLASH section 0x40030000



typedef struct SensortagFLASH_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t CFG;
    uint64_t FPAC1;
} SensortagFLASH_state;

static uint64_t SensortagFLASH_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagFLASH_state *s = (SensortagFLASH_state *)opaque;

    switch(offset){
    case 0x24:
        return s->CFG;
    case 0x2048:
        return s->FPAC1;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagFLASH_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagFLASH_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagFLASH_state *s = (SensortagFLASH_state *)opaque;
    switch(offset){
    case 0x24:
        s->CFG = value;
        break;
    case 0x2048:
        s->FPAC1 = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagFLASH_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
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
    s->CFG = 0x80000125;
    s->FPAC1 = 0x01392081;
}

static Property SensortagFLASH_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagFLASH_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagFLASH_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagFLASH_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
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




//CCFG section 0x50003000


typedef struct SensortagCCFG_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t SIZE_AND_DIS_FLAGS;
} SensortagCCFG_state;

static uint64_t SensortagCCFG_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagCCFG_state *s = (SensortagCCFG_state *)opaque;

    switch(offset){
    case 0xfb0:
        return s->SIZE_AND_DIS_FLAGS;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagCCFG_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagCCFG_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagCCFG_state *s = (SensortagCCFG_state *)opaque;
    switch(offset){
    case 0xfb0:
        s->SIZE_AND_DIS_FLAGS = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagCCFG_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagCCFG_ops = {
    .read = SensortagCCFG_read, 
    .write = SensortagCCFG_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagCCFG "SensortagCCFG"

#define SensortagCCFG(obj) \
    OBJECT_CHECK(SensortagCCFG_state, (obj), Type_SensortagCCFG)



static void SensortagCCFG_realize(DeviceState *dev, Error **errp)
{
    SensortagCCFG_state *s = SensortagCCFG(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagCCFG_ops, s,
                          "CCFG", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->SIZE_AND_DIS_FLAGS = 0x0058FFFD;
}

static Property SensortagCCFG_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagCCFG_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagCCFG_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagCCFG_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagCCFG_properties;

}


static const TypeInfo SensortagCCFG_info = {
    .name          = Type_SensortagCCFG,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagCCFG_state),
    .class_init    = SensortagCCFG_class_init, 
};

static void SensortagCCFG_register_types(void)
{
    type_register_static(&SensortagCCFG_info);
}

type_init(SensortagCCFG_register_types)




//VIMS section 0x40034000 Versatile Instruction Memory System Control 



typedef struct SensortagVIMS_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t STAT;
    uint64_t CTL;
} SensortagVIMS_state;

static uint64_t SensortagVIMS_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagVIMS_state *s = (SensortagVIMS_state *)opaque;

    switch(offset){
    case 0x0:
        return s->STAT;
    case 0x4:
        return s->CTL;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagVIMS_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagVIMS_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagVIMS_state *s = (SensortagVIMS_state *)opaque;
    switch(offset){
    case 0x0:
        s->STAT = value;
        break;
    case 0x4:
        s->CTL = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagVIMS_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagVIMS_ops = {
    .read = SensortagVIMS_read, 
    .write = SensortagVIMS_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagVIMS "SensortagVIMS"

#define SensortagVIMS(obj) \
    OBJECT_CHECK(SensortagVIMS_state, (obj), Type_SensortagVIMS)



static void SensortagVIMS_realize(DeviceState *dev, Error **errp)
{
    SensortagVIMS_state *s = SensortagVIMS(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagVIMS_ops, s,
                          "VIMS", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->STAT = 0x1;
    s->CTL = 0x20000005;
}

static Property SensortagVIMS_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagVIMS_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagVIMS_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagVIMS_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagVIMS_properties;

}


static const TypeInfo SensortagVIMS_info = {
    .name          = Type_SensortagVIMS,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagVIMS_state),
    .class_init    = SensortagVIMS_class_init, 
};

static void SensortagVIMS_register_types(void)
{
    type_register_static(&SensortagVIMS_info);
}

type_init(SensortagVIMS_register_types)




//PRCM section 0x40082000


typedef struct SensortagPRCM_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t WARMRESET;
    uint64_t PDCTL1VIMS;    
} SensortagPRCM_state;

static uint64_t SensortagPRCM_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagPRCM_state *s = (SensortagPRCM_state *)opaque;

    switch(offset){
    case 0x110:
        return s->WARMRESET;
    case 0x18c:
        return s->PDCTL1VIMS;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagPRCM_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagPRCM_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagPRCM_state *s = (SensortagPRCM_state *)opaque;
    switch(offset){
    case 0x110:
        s->WARMRESET = value;
        break;
    case 0x18c:
        s->PDCTL1VIMS = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagPRCM_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagPRCM_ops = {
    .read = SensortagPRCM_read, 
    .write = SensortagPRCM_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagPRCM "SensortagPRCM"

#define SensortagPRCM(obj) \
    OBJECT_CHECK(SensortagPRCM_state, (obj), Type_SensortagPRCM)



static void SensortagPRCM_realize(DeviceState *dev, Error **errp)
{
    SensortagPRCM_state *s = SensortagPRCM(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagPRCM_ops, s,
                          "PRCM", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->WARMRESET = 0x4;
    s->PDCTL1VIMS = 0x0;
}

static Property SensortagPRCM_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagPRCM_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagPRCM_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagPRCM_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagPRCM_properties;

}


static const TypeInfo SensortagPRCM_info = {
    .name          = Type_SensortagPRCM,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagPRCM_state),
    .class_init    = SensortagPRCM_class_init, 
};

static void SensortagPRCM_register_types(void)
{
    type_register_static(&SensortagPRCM_info);
}

type_init(SensortagPRCM_register_types)



//AUX_WUC section 0x400C6000

typedef struct SensortagAUX_WUC_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t size; // size of mimo section. (argument transfered from CC26xx.c)
    uint64_t MODCLKEN1;// 0x5c
    uint64_t PWROFFREQ;// 0x4
    uint64_t PWRDWNREQ;// 0x8
    uint64_t CLKLFREQ;// 0x10
    uint64_t CLKLFACK;// 0x14
} SensortagAUX_WUC_state;

static uint64_t SensortagAUX_WUC_read(void *opaque, hwaddr offset,
                                   unsigned size)
{    
    //uint64_t r;
    SensortagAUX_WUC_state *s = (SensortagAUX_WUC_state *)opaque;

    switch(offset){
    case 0x4:
        return s->PWROFFREQ;
    case 0x8:
        return s->PWRDWNREQ;
    case 0x10:
        return s->CLKLFREQ;
    case 0x14:
        return s->CLKLFACK;

    case 0x5c:
        return s->MODCLKEN1;
    default:
        qemu_log_mask(LOG_UNIMP,
                      "SensortagAUX_WUC_read_unimplemented: Bad offset %x\n", (int)offset);
        return 0x0;
    }
}

static void SensortagAUX_WUC_write(void *opaque, hwaddr offset, uint64_t value,
                                   unsigned size)
{   
    SensortagAUX_WUC_state *s = (SensortagAUX_WUC_state *)opaque;
    switch(offset){
    case 0x4:
        s->PWROFFREQ = value;
        break;
    case 0x8:
        s->PWRDWNREQ = value;
        break;
    case 0x10:
        s->CLKLFREQ = value;
        qemu_log_mask(LOG_UNIMP,
                      "************hey,did set offset %x Write value %x\n", (int)offset,(int)value);
        s->CLKLFACK = 0x1; 
        /*  17.6.1.1 Active Mode
        1. Set the AUX_WUC:CLKLFREQ.REQ register high.
        2. Wait for the AUX_WUC:CLKLFACK.ACK register to go high.
        3. Set the AUX_WUC:CLKLFREQ.REQ register low.
        4. Wait for the AUX_WUC:CLKLFACK.ACK register to go low.
        */ 
        break;
    case 0x14:
        s->CLKLFACK = value;
        break;
    case 0x5c:
        s->MODCLKEN1 = value;
        break;
    default:
    qemu_log_mask(LOG_UNIMP,
                      "SensortagAUX_WUC_write_unimplemented: Bad offset %x Write value %x\n", (int)offset,(int)value);
    }
}

static const MemoryRegionOps SensortagAUX_WUC_ops = {
    .read = SensortagAUX_WUC_read, 
    .write = SensortagAUX_WUC_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

#define Type_SensortagAUX_WUC "SensortagAUX_WUC"

#define SensortagAUX_WUC(obj) \
    OBJECT_CHECK(SensortagAUX_WUC_state, (obj), Type_SensortagAUX_WUC)



static void SensortagAUX_WUC_realize(DeviceState *dev, Error **errp)
{
    SensortagAUX_WUC_state *s = SensortagAUX_WUC(dev);

    if (s->size == 0) {
        error_setg(errp, "property 'size' not specified or zero");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(s), &SensortagAUX_WUC_ops, s,
                          "AUX_WUC", s->size);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    s->MODCLKEN1 = 0x1;
}

static Property SensortagAUX_WUC_properties[] = {
    DEFINE_PROP_UINT64("size", SensortagAUX_WUC_state, size, 0),
    DEFINE_PROP_END_OF_LIST(),
};


static void SensortagAUX_WUC_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = SensortagAUX_WUC_realize;// realize is instanciate?, similar func as add a .instance_init. in Typeinfo
    dc->props = SensortagAUX_WUC_properties;

}


static const TypeInfo SensortagAUX_WUC_info = {
    .name          = Type_SensortagAUX_WUC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SensortagAUX_WUC_state),
    .class_init    = SensortagAUX_WUC_class_init, 
};

static void SensortagAUX_WUC_register_types(void)
{
    type_register_static(&SensortagAUX_WUC_info);
}

type_init(SensortagAUX_WUC_register_types)




//FCFG section

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
