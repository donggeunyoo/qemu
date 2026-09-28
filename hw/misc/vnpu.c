/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/units.h"
#include "hw/pci/msi.h"
#include "hw/pci/pci_device.h"

#define TYPE_VNPU "vnpu"
OBJECT_DECLARE_SIMPLE_TYPE(VnpuState, VNPU)

#define VNPU_REG_ID         0x00
#define VNPU_REG_IRQ_STATUS 0x04
#define VNPU_REG_IRQ_RAISE  0x08
#define VNPU_ID             0x564e5055

struct VnpuState {
    PCIDevice parent_obj;
    MemoryRegion mmio;
    uint32_t irq_status;
};

static uint64_t vnpu_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    VnpuState *s = opaque;

    switch (addr) {
    case VNPU_REG_ID:
        return VNPU_ID;
    case VNPU_REG_IRQ_STATUS:
        return s->irq_status;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "vnpu: read from unknown register 0x%" HWADDR_PRIx "\n",
                      addr);
        return 0;
    }
}

static void vnpu_mmio_write(void *opaque, hwaddr addr, uint64_t data,
                            unsigned size)
{
    VnpuState *s = opaque;
    PCIDevice *pdev = PCI_DEVICE(s);

    switch (addr) {
    case VNPU_REG_IRQ_STATUS:
        s->irq_status &= ~data;
        break;
    case VNPU_REG_IRQ_RAISE:
        s->irq_status |= data;
        if (s->irq_status && msi_enabled(pdev)) {
            msi_notify(pdev, 0);
        }
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "vnpu: write to 0x%" HWADDR_PRIx " ignored\n", addr);
        break;
    }
}

static const MemoryRegionOps vnpu_mmio_ops = {
    .read = vnpu_mmio_read,
    .write = vnpu_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void vnpu_realize(PCIDevice *pdev, Error **errp)
{
    VnpuState *s = VNPU(pdev);

    if (msi_init(pdev, 0, 1, true, false, errp)) {
        return;
    }

    memory_region_init_io(&s->mmio, OBJECT(s), &vnpu_mmio_ops, s,
                          "vnpu-mmio", 4 * KiB);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &s->mmio);
}

static void vnpu_exit(PCIDevice *pdev)
{
    msi_uninit(pdev);
}

static void vnpu_reset_enter(Object *obj, ResetType type)
{
    VnpuState *s = VNPU(obj);

    s->irq_status = 0;
}

static void vnpu_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);

    k->realize = vnpu_realize;
    k->exit = vnpu_exit;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0x4e50;
    k->class_id = PCI_CLASS_ACCELERATOR_PROCESSING;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    rc->phases.enter = vnpu_reset_enter;
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
