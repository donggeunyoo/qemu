/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci_device.h"

#define TYPE_VNPU "vnpu"
OBJECT_DECLARE_SIMPLE_TYPE(VnpuState, VNPU)

struct VnpuState {
    PCIDevice parent_obj;
    MemoryRegion mmio;
};

static void vnpu_realize(PCIDevice *pdev, Error **errp)
{
    VnpuState *s = VNPU(pdev);

    memory_region_init_io(&s->mmio, OBJECT(s), NULL, s, "vnpu-mmio", 4 * KiB);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &s->mmio);
}

static void vnpu_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

    k->realize = vnpu_realize;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0x4e50;
    k->class_id = PCI_CLASS_ACCELERATOR_PROCESSING;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo vnpu_types[] = {
    {
        .name          = TYPE_VNPU,
        .parent        = TYPE_PCI_DEVICE,
        .instance_size = sizeof(VnpuState),
        .class_init    = vnpu_class_init,
        .interfaces    = (const InterfaceInfo[]) {
            { INTERFACE_CONVENTIONAL_PCI_DEVICE },
            { },
        },
    },
};

DEFINE_TYPES(vnpu_types)
