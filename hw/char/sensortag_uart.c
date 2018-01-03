/*
 * TI sensortag uart (suart)
 *
 * Copyright (c) 2006 CodeSourcery.
 * Written by Paul Brook
 *
 * This code is licensed under the GPL.
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "chardev/char-fe.h"
#include "qemu/log.h"
#include "trace.h"

#define TYPE_SUART "suart"
#define SUART(obj) OBJECT_CHECK(SUARTState, (obj), TYPE_SUART)

static const MemoryRegionOps suart_ops = {
    .read = suart_read,
    .write = suart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void suart_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    SUARTState *s = SUART(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &suart_ops, s, "suart", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    s->read_trigger = 1;
    s->ifl = 0x12;
    s->cr = 0x300;
    s->flags = 0x90;

    s->id = suart_id;
}

static void suart_realize(DeviceState *dev, Error **errp)
{
    PL011State *s = SUART(dev);

    /* void qemu_chr_fe_set_handlers(CharBackend *b,
                              IOCanReadHandler *fd_can_read,
                              IOReadHandler *fd_read,
                              IOEventHandler *fd_event,
                              BackendChangeHandler *be_change,
                              void *opaque,
                              GMainContext *context,
                              bool set_open)
    */

    qemu_chr_fe_set_handlers(&s->chr, suart_can_receive, suart_receive,
                             suart_event, NULL, s, NULL, true);
}

static void suart_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = suart_realize;
    dc->vmsd = &vmstate_suart;
    dc->props = suart_properties;
}


static const TypeInfo suart_info = {
    .name          = TYPE_SUART,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SUARTState),
    .instance_init = suart_init,
    .class_init    = suart_class_init,
};

static void suart_register_types(void)
{
    type_register_static(&suart_info);
}

type_init(suart_register_types)