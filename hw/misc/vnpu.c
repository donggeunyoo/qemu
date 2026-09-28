/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "hw/pci/pci_device.h"

#define TYPE_VNPU "vnpu"

static void vnpu_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0x4e50;
    k->class_id = PCI_CLASS_ACCELERATOR_PROCESSING;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo vnpu_types[] = {
    {
        .name          = TYPE_VNPU,
        .parent        = TYPE_PCI_DEVICE,
        .class_init    = vnpu_class_init,
        .interfaces    = (const InterfaceInfo[]) {
            { INTERFACE_CONVENTIONAL_PCI_DEVICE },
            { },
        },
    },
};

DEFINE_TYPES(vnpu_types)
