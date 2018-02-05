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

/* General purpose timer module.  */

#define TYPE_SAON_RTC "saon_rtc"
#define SAON_RTC(obj) \
    OBJECT_CHECK(saon_rtc_state, (obj), TYPE_SAON_RTC)

typedef struct saon_rtc_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint32_t ctl; // 0x0 control
    uint32_t evflags; //0x4 event flag & rtc status
    uint32_t sec; //0x8 second counter value, int part
    uint32_t subsec; //0xc second counter value, frac part
    uint32_t chctl; //0x14 chann config
    uint32_t ch0cmp; // chann 0 compare val
    uint32_t sync; //aon sync
    int64_t tick;
    struct saon_rtc_state *opaque;
    QEMUTimer *timer;
    qemu_irq irq;
} saon_rtc_state;

static void saon_rtc_update_irq(saon_rtc_state *s)
{
    int level;
    level = (s->state & s->mask) != 0;
    qemu_set_irq(s->irq, level);
}

static void saon_rtc_stop(gptm_state *s, int n)
{
    timer_del(s->timer[n]);
}

static void saon_rtc_reload(gptm_state *s, int n, int reset)
{
    int64_t tick;
    if (reset)
        tick = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    else
        tick = s->tick[n];

    if (s->config == 0) {
        /* 32-bit CountDown.  */
        uint32_t count;
        count = s->load[0] | (s->load[1] << 16);
        tick += (int64_t)count * system_clock_scale;
    } else if (s->config == 1) {
        /* 32-bit RTC.  1Hz tick.  */
        tick += NANOSECONDS_PER_SECOND;
    } else if (s->mode[n] == 0xa) {
        /* PWM mode.  Not implemented.  */
    } 
    s->tick[n] = tick;
    timer_mod(s->timer, tick);

}

static void saon_rtc_tick(void *opaque)
{
    saon_rtc_state *s = (saon_rtc_state *)opaque;

    if (s->config == 0) {
        s->state |= 1;
        if (s->mode[0] & 1) {
            /* One-shot.  */
            s->control &= ~1;
        } else {
            /* Periodic.  */
            saon_rtc_reload(s, 0, 0);
        }
    } else if (s->config == 1) {
        /* RTC.  */
        uint32_t match;
        s->rtc++;
        match = s->match[0] | (s->match[1] << 16);
        if (s->rtc > match)
            s->rtc = 0;
        if (s->rtc == 0) {
            s->state |= 8;
        }
        saon_rtc_reload(s, 0, 0);
    } else if (s->mode[n] == 0xa) {
        /* PWM mode.  Not implemented.  */
    } else {
        qemu_log_mask(LOG_UNIMP,
                      "GPTM: 16-bit timer mode unimplemented: 0x%x\n",
                      s->mode[n]);
    }
    saon_rtc_update_irq(s);
}

static uint64_t saon_rtc_read(void *opaque, hwaddr offset,
                          unsigned size)
{
    saon_rtc_state *s = (saon_rtc_state *)opaque;

/*  uint32_t ctl; // 0x0 control
    uint32_t evflags; //0x4 event flag & rtc status
    uint32_t sec; //0x8 second counter value, int part
    uint32_t subsec; //0xc second counter value, frac part
    uint32_t chctl; //0x14 chann config
    uint32_t ch0cmp; //0x18 chann 0 compare val
    uint32_t sync; //0x2c aon sync
*/


    switch (offset) {
    case 0x00: 
        return s->ctl;
    case 0x04: 
        return s->evflags;
    case 0x08: 
        return s->sec;
    case 0x0c: 
        return s->subsec;
    case 0x14: 
        return s->chctl;
    case 0x18: 
        return s->ch0cmp;
    case 0x2c: 
        return s->sync;
    default:
        qemu_log_mask(LOG_UNIMP,
            "saon_rtc_read_unimplemented: Offset %x \n", (int)offset);
        return 0;
    }
}

static void saon_rtc_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned size)
{
    saon_rtc_state *s = (saon_rtc_state *)opaque;

    /* uint32_t ctl; // 0x0 control
    uint32_t evflags; //0x4 event flag & rtc status
    uint32_t sec; //0x8 second counter value, int part
    uint32_t subsec; //0xc second counter value, frac part
    uint32_t chctl; //0x14 chann config
    uint32_t ch0cmp; //0x18 chann 0 compare val
    uint32_t sync; //0x2c aon sync
*/

    switch (offset) {
    case 0x00: 
        s->ctl = value;
        // 0x80 reset counter
        if (value == 0x80) {
        	s->sec = 0;
        	s->subsec = 0;
      	    timer_mod(s->timer, 0);
        }
        break;
    case 0x04: 
        s->evflags = value;
        break;
    case 0x14: 
        s->chctl = value;
        break;
    case 0x18: 
        s->ch0cmp = value;
        break;
    case 0x2c: 
        s->sync = value;
        break;
    default:
        qemu_log_mask(LOG_UNIMP,
            "saon_rtc_write_unimplemented: Offset %x Write value %x\n", (int)offset,(int)value);
    }
    //saon_rtc_update_irq(s);
}

static const MemoryRegionOps gptm_ops = {
    .read = gptm_read,
    .write = gptm_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_saon_rtc = {
    .name = "saon_rtc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(config, saon_rtc_state),
        VMSTATE_UINT32_ARRAY(mode, saon_rtc_state, 2),
        VMSTATE_UINT32(control, saon_rtc_state),
        VMSTATE_UINT32(state, saon_rtc_state),
        VMSTATE_UINT32(mask, saon_rtc_state),
        VMSTATE_UNUSED(8),
        VMSTATE_UINT32_ARRAY(load, saon_rtc_state, 2),
        VMSTATE_UINT32_ARRAY(match, saon_rtc_state, 2),
        VMSTATE_UINT32_ARRAY(prescale, saon_rtc_state, 2),
        VMSTATE_UINT32_ARRAY(match_prescale, saon_rtc_state, 2),
        VMSTATE_UINT32(rtc, saon_rtc_state),
        VMSTATE_INT64_ARRAY(tick, saon_rtc_state, 2),
        VMSTATE_TIMER_PTR_ARRAY(timer, saon_rtc_state, 2),
        VMSTATE_END_OF_LIST()
    }
};

static void saon_rtc_init(Object *obj)
{
    DeviceState *dev = DEVICE(obj);
    saon_rtc_state *s = SAON_RTC_INIT(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    sysbus_init_irq(sbd, &s->irq);

    memory_region_init_io(&s->iomem, obj, &gptm_ops, s,
                          "saon_rtc", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->opaque = s;
    s->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, saon_rtc_tick, &s->opaque);
}



static void saon_rtc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_saon_rtc;
}

static const TypeInfo saon_rtc_info = {
    .name          = TYPE_SAON_RTC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(saon_rtc_state),
    .instance_init = saon_rtc_init,
    .class_init    = saon_rtc_class_init,
};

static void saon_rtc_register_types(void)
{
    type_register_static(&saon_rtc_info);
}

type_init(saon_rtc_register_types)