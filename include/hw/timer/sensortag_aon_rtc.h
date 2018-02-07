/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_sensortag_aon_rtc
#define HW_sensortag_aon_rtc

static inline DeviceState *saon_rtc_create(hwaddr addr,
                                        qemu_irq irq)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, "saon_rtc");
    s = SYS_BUS_DEVICE(dev);
    qdev_init_nofail(dev);
    sysbus_mmio_map(s, 0, addr);
    sysbus_connect_irq(s, 0, irq);

    return dev;
}


/*
static void stm32_create_timer_dev(
        Object *stm32_container,
        stm32_periph_t periph,
        int timer_num,
        DeviceState *rcc_dev,
        DeviceState **gpio_dev,
        DeviceState *afio_dev,
        hwaddr addr,
        qemu_irq *irq,
        int num_irqs)
{
    int i;
    char child_name[9];
    DeviceState *timer_dev = qdev_create(NULL, "stm32-timer");
    QDEV_PROP_SET_PERIPH_T(timer_dev, "periph", periph);
    qdev_prop_set_ptr(timer_dev, "stm32_rcc", rcc_dev);
    qdev_prop_set_ptr(timer_dev, "stm32_gpio", gpio_dev);
    qdev_prop_set_ptr(timer_dev, "stm32_afio", afio_dev);
    snprintf(child_name, sizeof(child_name), "timer[%i]", timer_num);
    object_property_add_child(stm32_container, child_name, OBJECT(timer_dev), NULL);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    if (irq) {
        sysbus_connect_irq(SYS_BUS_DEVICE(dev), 0, irq);
    }
    return dev;
}

*/
#endif
