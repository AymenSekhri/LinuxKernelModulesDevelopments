# USB
## Low Level USB
references: 
https://wiki.osdev.org/USB 
http://www.usbmadesimple.co.uk/ums_7.htm
### Transfers
**Control Transfers** provide lossless transmissions and are used to configure a device. Thus, all USB devices must support control transfers at least via the default control pipe.
**Bulk Data Transfers** provide lossless, sequential transmissions and are typically used to transfer large amounts of data.
**Interrupt Data Transfers** provide reliable, limited-latency transmissions typically needed by human input devices such as a mouse or a joystick.
**Isochronous Data Transfers**, also called Streaming Real-time Transfers, negotiate a required bandwidth and latency when initializing the transfer. This transfer type is predominantly used for such applications as streaming audio. Since data-delivery rate is considered more important than data integrity for this type of transfer, it does not provide any type of error checking or correction mechanism.
### Transactions
Each Transfer has one or more transactions.
#### Control Transfer's Transactions
  * **SETUP:** initializes both the function's and the host's data toggle bits to 0.
  * **DATA:** This is optional, and can be more than one of it in the same direction.
  * **STATUS:** acknowledge the transfer (not the packet, as in acknowledging the configuration is done) which is single IN or a single OUT transaction, with DATA1 PID. if no DATA is not present it's IN transaction, if DATA is present then the STATUS is opposite direction of DATA.
#### Bulk and Interrupt Transfer's Transactions
They are both the same except that Bulk may responds with **NYET** packet.
They have only one transaction which is either IN our OUT.
#### Isochronous' Transactions
Consists of only one transaction either IN our OUT. but the transaction doesn't have handshake packet.

#### PING Transaction
FIXME: PING is separate transaction or packet in bulk ?
**PING** transaction is Control Transfer, When **PING** transaction is sent to the function each time want to write/read data. the function may responds with **NAK** packet (in PING transaction) which means that the function is not ready yet, or by **ACK** which means that you can read/write data.<br>
During bulk transactions, when **NYET** packet is received (inside the bulk transaction) instead of **ACK**, that means that the data is received the function, can't handle more data and you should send PING transaction to poll the state in the future.
### Packets
Each transaction consists of some packets which are atomic
* **Token Packet:** Used in all transactions.
* **Data Packet:** Used only of transactions that needs data.
**DATA0** and **DATA1** PIDs are used in Low and Full speed links as part of an error-checking system. When used, all data packets on a particular endpoint use an alternating DATA0 / DATA1 so that the endpoint knows if a received packet is the one it is expecting. If it is not it will still acknowledge (ACK) the packet as it is correctly received, but will then discard the data, assuming that it has been re-sent because the host missed seeing the ACK the first time it sent the data packet.
* **Handshake Packet:** Used for all transactions except Isochronous.
* **SOF Packet:** Not used any transaction except SOF which is in the start of each (micro)frame (more about (micro)frame in the next section).<br>
The Start of Frame packet is sent every 1 ms on full speed links. The frame is used as a time frame in which to schedule the data transfers which are required. For example, an isochronous endpoint will be assigned one transfer per frame.
* **Split Packet:** The SPLIT packet is the first packet in either a Start Split transaction or a Complete Split transaction, sent to a high speed hub when it is handling a low or full speed device.
### (Micro)Frames
USB timing consists of periodic parts of 1ms(microframes for low speed) and 125us(frames for high speed), each (micro)frame starts with SOF Transaction witch is one packet, then transactions different control transaction may be reordered (transactions of different transfers are reordered but they still maintain the order of the transactions of the same transfer).

## USB In Linux
### USB Devices in sysfs Tree
`sysfs` file system is used to view the devices tree, for example a USB mouse interface has the following path.<br>
`/sys/devices/pci0000:00/0000:00:09.0/usb2/2-1/2-1:1.0`
`/sys/devices             /pci0000:00       /0000:00:09.0     /usb2           /2-1                /2-1:1.0`
  ↑-> the file system        ↑-> PCI bus      ↑-> PCI slot      ↑-> USB hub     ↑-> USB Device      ↑-> Configuration . Interface
  <br>
USB sysfs device naming scheme is: `root_hub-hub_port:config.interface`

## Using URBs
#### #include <linux/usb.h>
Header file where everythingrelated to USB resides. It must be included by all USB device drivers.
#### struct usb_driver;
Structure that describes a USB driver.
#### struct usb_device_id;
Structure that describes the types of USB devices this driver supports.
#### int usb_register(struct usb_driver *d);
#### void usb_deregister(struct usb_driver *d);
Functions used to register and unregister a USB driver from the USB core.
#### struct usb_device *interface_to_usbdev(struct usb_interface *intf);
Retrieves the controlling struct usb_device * out of a struct usb_interface *.
#### struct usb_device;
Structure that controls an entire USB device.
#### struct usb_interface;
Main USB device structure that all USB drivers use to communicate with the USB core.
#### void usb_set_intfdata(struct usb_interface *intf, void *data);
#### void *usb_get_intfdata(struct usb_interface *intf);
Functions to set and get access to the private data pointer section within the
#### struct usb_interface.
#### struct usb_class_driver;
A structure that describes a USB driver that wants to use the USB major number to communicate with user-space programs.
#### int usb_register_dev(struct usb_interface *intf, struct usb_class_driver *class_driver);
#### void usb_deregister_dev(struct usb_interface *intf, struct usb_class_driver *class_driver);
Functions used to register and unregister a specific struct usb_interface * structure with a struct usb_class_driver * structure.
#### struct urb;
Structure that describes a USB data transmission.
#### struct urb *usb_alloc_urb(int iso_packets, int mem_flags);
#### void usb_free_urb(struct urb *urb);
Functions used to create and destroy a struct usb urb *.
#### int usb_submit_urb(struct urb *urb, int mem_flags);
#### int usb_kill_urb(struct urb *urb);
#### int usb_unlink_urb(struct urb *urb);
Functions used to start and stop a USB data transmission.
#### void usb_fill_int_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context, int interval);
#### void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context);
#### void usb_fill_control_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, unsigned char *setup_packet, void *transfer_buffer, int buffer_ length, usb_complete_t complete, void *context);
Functions used to initialize a struct urb before it is submitted to the USB core.
#### int usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe, void *data,
#### int len, int *actual_length, int timeout);
#### int usb_control_msg(struct usb_device *dev, unsigned int pipe, __u8 request, __u8 requesttype, __u16 value, __u16 index, void *data, __u16 size, int timeout);
Functions used to send or receive USB data without having to use a struct urb.