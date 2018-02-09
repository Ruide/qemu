/*
 * TI sensortag aon_rtc
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

#define NANOSECONDS_PER_SECOND 1000000000LL

#define SAON_RTC(obj) \
    OBJECT_CHECK(saon_rtc_state, (obj), "saon_rtc")

typedef struct saon_rtc_state {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint32_t ctl; // 0x0 control
    uint32_t evflags; //0x4 event flag & rtc status
    uint32_t sec; //0x8 second counter value, int part
    uint32_t subsec; //0xc second counter value, frac part
    uint32_t chctl; //0x14 chann config
    uint32_t ch0cmp; // chann 0 compare val
    uint32_t aon_sync; //aon sync
    int64_t tick;
    uint32_t irq_state;
    struct saon_rtc_state *opaque;
    QEMUTimer *timer;
    qemu_irq irq;
} saon_rtc_state;

/*

static void saon_rtc_update_irq(saon_rtc_state *s)
{
    int level;
    level = (s->irq_state != 0);
    qemu_set_irq(s->irq, level);
}

*/
//static void saon_rtc_stop(saon_rtc_state *s, int n)
//{
 //   timer_del(s->timer[n]);
//}

/*

static void saon_rtc_reload(saon_rtc_state *s, int reset)
{

    int64_t tick;
    if (reset)
        tick = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    else
        tick = s->tick;
    qemu_log_mask(LOG_UNIMP, "current_time: %d nanosecond\n", (int)tick);
    //tick += NANOSECONDS_PER_SECOND; 1000000000LL
    tick += (s->ch0cmp & 0xffff) * 15258;//select latter 16 bits 
    s->tick = tick;
    qemu_log_mask(LOG_UNIMP, "setted_callback_time: %d nanosecond, s->ch0cmp is %d\n", (int)tick,(int)s->ch0cmp);

    //modify the current timer so that it will be fired when current_time
    //>= expire_time. The corresponding callback will be called. 
	//void timer_mod_ns(QEMUTimer *ts, int64_t expire_time)
    timer_mod(s->timer, tick);// tick is expire time, when current time >= expired time, it fire callback
    //qemu_log_mask(LOG_UNIMP, "new_tick_after_plus_1_sec: %x\n", (int)qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL));
    //qemu_log_mask(LOG_UNIMP, "2nd_new_tick_after_plus_1_sec: %x\n", (int)qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL));
    saon_rtc_update_irq(s);

}

*/

static void saon_rtc_tick(void *opaque)
{
	int64_t tick;
    saon_rtc_state *s = (saon_rtc_state *)opaque;
    tick = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);

    qemu_log_mask(LOG_UNIMP, "1.tick_callback_time: %d nanosecond, s->ch0cmp: %d ns, s->ctl: %d \n", (int)tick,(int)s->ch0cmp,(int)s->ctl);

    if((int)s->ch0cmp == 0){
        return;
    }

    qemu_log_mask(LOG_UNIMP, "2.tick_callback_time: %d nanosecond, s->ch0cmp: %d ns\n", (int)tick,(int)s->ch0cmp);
    //15258.78 ns is the min unit for [0~15] bit of 32 bit time compare register
    tick = s->tick;
    tick += (s->ch0cmp & 0xffff) * 15258;//select latter 16 bits 
    s->tick = tick;
    timer_mod(s->timer, tick);// tick is expire time, when current time >= expired time, it fire callback

    int level;
    level = (s->irq_state == 0);
    qemu_set_irq(s->irq, level);
    //qemu_notify_event();

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

//    qemu_log_mask(LOG_UNIMP,
//            "saon_rtc_read_unimplemented: Offset %x\n", (int)offset);

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
        return s->aon_sync;
    default:
        qemu_log_mask(LOG_UNIMP,
            "saon_rtc_read_unimplemented: Offset %x\n", (int)offset);
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

//	qemu_log_mask(LOG_UNIMP,
//            "saon_rtc_write_unimplemented: Offset %x Write value %x\n", (int)offset,(int)value);


    switch (offset) {
    case 0x00: 
        s->ctl = value;
        // 0x80 reset counter
        if (value == 0x80) {
        	s->sec = 0;
        	s->subsec = 0;
      	    //saon_rtc_reload(s,1);
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
        qemu_log_mask(LOG_UNIMP, "write compare time: %d ns\n", (int)s->ch0cmp);
        //saon_rtc_reload(s,1);
        saon_rtc_tick(s);
        break;
    case 0x2c: 
        s->aon_sync = value;
        break;
    default:
        qemu_log_mask(LOG_UNIMP,
            "saon_rtc_write_unimplemented: Offset %x Write value %x\n", (int)offset,(int)value);
    }
    //saon_rtc_update_irq(s);
}

static const MemoryRegionOps saon_rtc_ops = {
    .read = saon_rtc_read,
    .write = saon_rtc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};




static const VMStateDescription vmstate_saon_rtc = {
    .name = "saon_rtc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(ctl, saon_rtc_state),
        VMSTATE_UINT32(evflags, saon_rtc_state),
        VMSTATE_UINT32(sec, saon_rtc_state),
        VMSTATE_UINT32(subsec, saon_rtc_state),
        VMSTATE_UINT32(chctl, saon_rtc_state),
        VMSTATE_UINT32(ch0cmp, saon_rtc_state),
        VMSTATE_UINT32(aon_sync, saon_rtc_state),
        VMSTATE_UINT32(irq_state, saon_rtc_state),
        VMSTATE_INT64(tick, saon_rtc_state),
        VMSTATE_TIMER_PTR(timer, saon_rtc_state),
        VMSTATE_END_OF_LIST()
    }
};

static void saon_rtc_init(Object *obj)
{
    //DeviceState *dev = DEVICE(obj);
    saon_rtc_state *s = SAON_RTC(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    sysbus_init_irq(sbd, &s->irq);

    memory_region_init_io(&s->iomem, obj, &saon_rtc_ops, s,
                          "saon_rtc", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->opaque = s;
    s->irq_state = 0;
    s->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, saon_rtc_tick, s->opaque);//should be s->opaque instead of &s
}



static void saon_rtc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_saon_rtc;
}

static const TypeInfo saon_rtc_info = {
    .name          = "saon_rtc",
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