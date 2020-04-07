# PCI
#### #include <linux/pci.h>
Header that includes symbolic names for the PCI registers and several vendor
and device ID values.
#### struct pci_dev;
Structure that represents a PCI device within the kernel.
#### struct pci_driver;
Structure that represents a PCI driver. All PCI drivers must define this.
#### struct pci_device_id;
Structure that describes the types of PCI devices this driver supports.
#### int pci_register_driver(struct pci_driver *drv);
#### int pci_module_init(struct pci_driver *drv);
#### void pci_unregister_driver(struct pci_driver *drv);
Functions that register or unregister a PCI driver from the kernel.
#### struct pci_dev *pci_find_device(unsigned int vendor, unsigned int device,
#### struct pci_dev *from);
#### struct pci_dev *pci_find_device_reverse(unsigned int vendor, unsigned int device, const struct pci_dev *from);
#### struct pci_dev *pci_find_subsys (unsigned int vendor, unsigned int device,
#### unsigned int ss_vendor, unsigned int ss_device, const struct pci_dev *from);
#### struct pci_dev *pci_find_class(unsigned int class, struct pci_dev *from);
Functions that search the device list for devices with a specific signature or those belonging to a specific class. The return value is NULL if none is found. from is used to continue a search; it must be NULL the first time you call either function, and it must point to the device just found if you are searching for more devices. These functions are not recommended to be used, use the pci_get_ variants instead.
#### struct pci_dev *pci_get_device(unsigned int vendor, unsigned int device, struct pci_dev *from);
#### struct pci_dev *pci_get_subsys(unsigned int vendor, unsigned int device,unsigned int ss_vendor, unsigned int ss_device, struct pci_dev *from);
#### struct pci_dev *pci_get_slot(struct pci_bus *bus, unsigned int devfn);
Functions that search the device list for devices with a specific signature or belonging to a specific class. The return value is NULL if none is found. from is used to continue a search; it must be NULL the first time you call either function, and it must point to the device just found if you are searching for more devices. The structure returned has its reference count incremented, and after the caller is finished with it, the function pci_dev_put must be called.

#### int pci_read_config_byte(struct pci_dev *dev, int where, u8 *val);
#### int pci_read_config_word(struct pci_dev *dev, int where, u16 *val);
#### int pci_read_config_dword(struct pci_dev *dev, int where, u32 *val);
#### int pci_write_config_byte (struct pci_dev *dev, int where, u8 *val);
#### int pci_write_config_word (struct pci_dev *dev, int where, u16 *val);
#### int pci_write_config_dword (struct pci_dev *dev, int where, u32 *val);
Functions that read or write a PCI configuration register. Although the Linux kernel takes care of byte ordering, the programmer must be careful about byte ordering when assembling multibyte values from individual bytes. The PCI bus is little-endian.
#### int pci_enable_device(struct pci_dev *dev);
Enables a PCI device.
#### unsigned long pci_resource_start(struct pci_dev *dev, int bar);
#### unsigned long pci_resource_end(struct pci_dev *dev, int bar);
#### unsigned long pci_resource_flags(struct pci_dev *dev, int bar);
Functions that handle PCI device resources.