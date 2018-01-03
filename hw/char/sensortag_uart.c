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

typedef struct SUARTState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint32_t readbuff;
    uint32_t flags;/*case 6 UARTFR fr */
    uint32_t lcr;/*case 11 UARTLCR_H lcrh*/
    uint32_t rsr;/*case 1 UARTRSR rsr/ecr*/
    uint32_t cr;/*case 12 UARTCR cr <=> ctl*/
    uint32_t dmacr;/*case 18 UARTDMACR dmacr*/
    uint32_t int_enabled;/*case 14 UARTIMSC imsc*/
    uint32_t int_level;/*case 15 UARTRIS ris*/
    uint32_t read_fifo[16]; /*case 0 UARTDR dr*/
    //uint32_t ilpr;/*case 8 UARTILPR */
    uint32_t ibrd;/*case 9 UARTIBRD ibrd*/
    uint32_t fbrd;/*case 10 UARTFBRD fbrd*/
    uint32_t ifl;/*case 13 UARTIFLS ifls */
    // miss: MIS, ICR 
    int read_pos;
    int read_count;
    int read_trigger;
    CharBackend chr;
    qemu_irq irq;
} SUARTState;

static void suart_update(SUARTState *s)
{
    uint32_t flags;

    flags = s->int_level & s->int_enabled; // imsc && ris
    trace_suart_irq_state(flags != 0); // flags != 0 set up the level in log()
    qemu_set_irq(s->irq, flags != 0);
}


static uint64_t suart_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    SUARTState *s = (SUARTState *)opaque;
    uint32_t c;
    uint64_t r;

    switch (offset >> 2) {
    case 0: /* UARTDR */
        s->flags &= ~(0x40); //#define PL011_FLAG_RXFF 0x40 receive fifo full
        c = s->read_fifo[s->read_pos];
        if (s->read_count > 0) {
            s->read_count--;
            if (++s->read_pos == 16)
                s->read_pos = 0;
        }
        if (s->read_count == 0) {
            s->flags |= 0x10; //#define PL011_FLAG_RXFE 0x10 0x40 receive fifo empty
        }
        if (s->read_count == s->read_trigger - 1)
            s->int_level &= ~ 0x10;//#define PL011_INT_TX 0x20
                            //#define PL011_INT_RX 0x10

        trace_suart_read_fifo(s->read_count);
        s->rsr = c >> 8;
        suart_update(s);
        qemu_chr_fe_accept_input(&s->chr);
        r = c;
        break;
    case 1: /* UARTRSR */
        r = s->rsr;
        break;
    case 6: /* UARTFR */
        r = s->flags;
        break;
    //case 8: /* UARTILPR */
    //    r = s->ilpr;
    //    break;
    case 9: /* UARTIBRD */
        r = s->ibrd;
        break;
    case 10: /* UARTFBRD */
        r = s->fbrd;
        break;
    case 11: /* UARTLCR_H */
        r = s->lcr;
        break;
    case 12: /* UARTCR */
        r = s->cr;
        break;
    case 13: /* UARTIFLS */
        r = s->ifl;
        break;
    case 14: /* UARTIMSC */
        r = s->int_enabled;
        break;
    case 15: /* UARTRIS */
        r = s->int_level;
        break;
    case 16: /* UARTMIS */
        r = s->int_level & s->int_enabled;
        break;
    case 18: /* UARTDMACR */
        r = s->dmacr;
        break;
    //case 0x3f8 ... 0x400:
    //    r = s->id[(offset - 0xfe0) >> 2];
    //    break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "suart_read: Bad offset %x\n", (int)offset);
        r = 0;
        break;
    }

    trace_suart_read(offset, r);
    return r;
}

static void suart_set_read_trigger(SUARTState *s)
{
#if 0
    /* The docs say the RX interrupt is triggered when the FIFO exceeds
       the threshold.  However linux only reads the FIFO in response to an
       interrupt.  Triggering the interrupt when the FIFO is non-empty seems
       to make things work.  */
    if (s->lcr & 0x10)
        s->read_trigger = (s->ifl >> 1) & 0x1c;
    else
#endif
        s->read_trigger = 1;
}

static void suart_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
    SUARTState *s = (SUARTState *)opaque;
    unsigned char ch;

    trace_suart_write(offset, value);

    switch (offset >> 2) {
    case 0: /* UARTDR */
        /* ??? Check if transmitter is enabled.  */
        ch = value;
        /* XXX this blocks entire thread. Rewrite to use
         * qemu_chr_fe_write and background I/O callbacks */
        qemu_chr_fe_write_all(&s->chr, &ch, 1);
        s->int_level |= 0x20; // #define PL011_INT_TX 0x20
                              // #define PL011_INT_RX 0x10

        suart_update(s);
        break;
    case 1: /* UARTRSR/UARTECR */
        s->rsr = 0;
        break;
    case 6: /* UARTFR */
        /* Writes to Flag register are ignored.  */
        break;
    //case 8: /* UARTUARTILPR */
    //    s->ilpr = value;
    //    break;
    case 9: /* UARTIBRD */
        s->ibrd = value;
        break;
    case 10: /* UARTFBRD */
        s->fbrd = value;
        break;
    case 11: /* UARTLCR_H */
        /* Reset the FIFO state on FIFO enable or disable */
        if ((s->lcr ^ value) & 0x10) {
            s->read_count = 0;
            s->read_pos = 0;
        }
        s->lcr = value;
        suart_set_read_trigger(s);
        break;
    case 12: /* UARTCR */
        /* ??? Need to implement the enable and loopback bits.  */
        s->cr = value;
        break;
    case 13: /* UARTIFS */
        s->ifl = value;
        suart_set_read_trigger(s);
        break;
    case 14: /* UARTIMSC */
        s->int_enabled = value;
        suart_update(s);
        break;
    case 17: /* UARTICR */
        s->int_level &= ~value;
        suart_update(s);
        break;
    case 18: /* UARTDMACR */
        s->dmacr = value;
        if (value & 3) {
            qemu_log_mask(LOG_UNIMP, "suart: DMA not implemented\n");
        }
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "suart_write: Bad offset %x\n", (int)offset);
    }
}



// MemoryRegionOps includes additional fields that are used to specify constraints/attributes that control the manner
// of host callback invocation. A detailed description is these fields is included in the MemoryRegionOps definition 
// (see include/exec/memory.h). These attributes take effect once the region becomes exposed to the guest.
static const MemoryRegionOps suart_ops = {
// .read callback is invoked whenever vCPU performs read op from guest addr space mapping of QEMU MMIO memory region
    .read = suart_read, 
// .write method gets executed upon vCPU writes to the region
    .write = suart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};



// IOCanReadHandler *fd_can_read,
static int suart_can_receive(void *opaque)
{
    SUARTState *s = (SUARTState *)opaque;
    int r;

    if (s->lcr & 0x10) {
        r = s->read_count < 16;
    } else {
        r = s->read_count < 1;
    }
    trace_suart_can_receive(s->lcr, s->read_count, r);
    return r;
}

static void suart_put_fifo(void *opaque, uint32_t value)
{
    SUARTState *s = (SUARTState *)opaque;
    int slot;

    slot = s->read_pos + s->read_count;
    if (slot >= 16)
        slot -= 16;
    s->read_fifo[slot] = value;// a buffer
    s->read_count++;
    s->flags &= ~(0x10);//PL011_FLAG_RXFE
    trace_suart_put_fifo(value, s->read_count);
    if (!(s->lcr & 0x10) || s->read_count == 16) {
        trace_suart_put_fifo_full();
        s->flags |= 0x40; // PL011_FLAG_RXFF
    }
    if (s->read_count == s->read_trigger) {
        s->int_level |= 0x10;// PL011_INT_RX
        suart_update(s);
    }
}


//IOReadHandler *fd_read,                            
static void suart_receive(void *opaque, const uint8_t *buf, int size)
{
    suart_put_fifo(opaque, *buf);
}

//IOEventHandler *fd_event,
static void suart_event(void *opaque, int event)
{
    if (event == CHR_EVENT_BREAK) // #define CHR_EVENT_BREAK   0 /* serial break char */
        suart_put_fifo(opaque, 0x400);
}

// for saving/loading state of the device
static const VMStateDescription vmstate_suart = {
    .name = "suart",
    .version_id = 2,
    .minimum_version_id = 2,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(readbuff, SUARTState),
        VMSTATE_UINT32(flags, SUARTState),
        VMSTATE_UINT32(lcr, SUARTState),
        VMSTATE_UINT32(rsr, SUARTState),
        VMSTATE_UINT32(cr, SUARTState),
        VMSTATE_UINT32(dmacr, SUARTState),
        VMSTATE_UINT32(int_enabled, SUARTState),
        VMSTATE_UINT32(int_level, SUARTState),
        VMSTATE_UINT32_ARRAY(read_fifo, SUARTState, 16),
        //VMSTATE_UINT32(ilpr, PL011State),
        VMSTATE_UINT32(ibrd, SUARTState),
        VMSTATE_UINT32(fbrd, SUARTState),
        VMSTATE_UINT32(ifl, SUARTState),
        VMSTATE_INT32(read_pos, SUARTState),
        VMSTATE_INT32(read_count, SUARTState),
        VMSTATE_INT32(read_trigger, SUARTState),
        VMSTATE_END_OF_LIST()
    }
};

// === Describing qdev data structures ===

//In addition to defining the structs, each bus and device should describe 
//the structs as "properties". 
// this property may be used for query the char device list ..

static Property suart_properties[] = {
    DEFINE_PROP_CHR("chardev", SUARTState, chr),
    DEFINE_PROP_END_OF_LIST(),
};


static void suart_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);//create a sys_bus_device object -> obj
    SUARTState *s = SUART(obj);//dynamic cast obj to SUARTState
                            //(can do so cause SUARTState is inherited from sys_bus_device)

// memory_region_init_io() constructor initializes a QEMU MMIO(Memory mapped IO) memory region.
/*    void memory_region_init_io(MemoryRegion *mr, 
                           struct Object *owner,
                           const MemoryRegionOps *ops,
                           void *opaque,
                           const char *name,
                           uint64_t size);
*/
    memory_region_init_io(&s->iomem, OBJECT(s), &suart_ops, s, "suart", 0x1000); //MemoryRegionOps suart_ops
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    s->read_trigger = 1;
    s->ifl = 0x12;
    s->cr = 0x300;
    s->flags = 0x90;

}

static void suart_realize(DeviceState *dev, Error **errp)
{
    SUARTState *s = SUART(dev);

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
                             // char backend as a file. So save the devicestate in a file
}

static void suart_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = suart_realize;// device class realize: implement the device state into a file
    dc->vmsd = &vmstate_suart;// virtual machine state description: saving/loading state of the device
    dc->props = suart_properties;// properties
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