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

#ifndef HW_sensortag_misc
#define HW_sensortag_misc

static inline DeviceState *SensortagFCFG_create(hwaddr addr,hwaddr size)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, "SensortagFCFG");
    s = SYS_BUS_DEVICE(dev);
    qdev_prop_set_uint64(dev, "size", size);// here SensortagFCFG_state has the member size as property. 
    //so here realize the property in the SensortagFCFG_state(device's state)
    qdev_init_nofail(dev);
    sysbus_mmio_map(s, 0, addr);// set up the mmio base and may set its priority if needed by add fouth argument

    return dev;
}


static inline DeviceState *Dummydevice_create(const char * name, hwaddr addr,hwaddr size)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, name);
    s = SYS_BUS_DEVICE(dev);
    qdev_prop_set_uint64(dev, "size", size);// here SensortagFCFG_state has the member size as property. 
    //so here realize the property in the SensortagFCFG_state(device's state)
    qdev_init_nofail(dev);
    sysbus_mmio_map(s, 0, addr);// set up the mmio base and may set its priority if needed by add fouth argument

    return dev;
}
// this create and init a device called "SensortagFCFG" and transfer the property size to it. 
// the implementation of the device is located at CC26xx_misc.c.

static inline DeviceState *create_empty_ram(const char * name, hwaddr addr,hwaddr size)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, name);
    s = SYS_BUS_DEVICE(dev);
    qdev_prop_set_uint64(dev, "size", size);// here SensortagFCFG_state has the member size as property. 
    //so here realize the property in the SensortagFCFG_state(device's state)
    qdev_init_nofail(dev);
    sysbus_mmio_map_overlap(s, 0, addr, -1000);// set up the mmio base and may set its priority if needed by add fouth argument

    return dev;
}

#endif
