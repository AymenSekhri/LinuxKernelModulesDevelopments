# USB
## Low Level USB
reference: https://wiki.osdev.org/USB 
### Transfers
**Control Transfers** provide lossless transmissions and are used to configure a device. Thus, all USB devices must support control transfers at least via the default control pipe.
**Bulk Data Transfers** provide lossless, sequential transmissions and are typically used to transfer large amounts of data.
**Interrupt Data Transfers** provide reliable, limited-latency transmissions typically needed by human input devices such as a mouse or a joystick.
**Isochronous Data Transfers**, also called Streaming Real-time Transfers, negotiate a required bandwidth and latency when initializing the transfer. This transfer type is predominantly used for such applications as streaming audio. Since data-delivery rate is considered more important than data integrity for this type of transfer, it does not provide any type of error checking or correction mechanism.
### Transaction
Each Transfer has one or more transactions.
#### Control Transfer's Transactions
  * **SETUP:** initializes both the function's and the host's data toggle bits to 0.
  * **DATA:** This is optional, and can be more than one of it in the same direction.
  * **STATUS:** acknowledge the transfer (not the packet, as in acknowledging the configuration is done) which is single IN or a single OUT transaction, with DATA1 PID. if no DATA is not present it's IN transaction, if DATA is present then the STATUS is opposite direction of DATA.
#### Bulk and Interrupt Transfer's Transactions
They are both the same except that Bulk may responds with **NYET** packet.

#### PING Transaction
When **PING** transaction is sent to the function each time want to write/read data. the function may responds with **NAK** packet (in PING transaction) which means that the function is not ready yet, or by **ACK** which means that you can read/write data.<br>
During bulk transactions, when **NYET** packet is received (inside the bulk transaction) instead of **ACK**, that means that the data is received the function, can't handle more data and you should send PING transaction to poll the state in the future.
### Packet
Each transaction consists of some packets which are atomic
* **Token Packet**
* **Data Packet**
* **Handshake Packet**

### (Micro)Frames
USB timing consists of periodic parts of 1ms(microframes for low speed) and 125us(frames for high speed), each (micro)frame starts with SOF Transaction witch is one packet, then transactions different control transaction may be reordered (transactions of different transfers are reordered but they still maintain the order of the transactions of the same transfer).