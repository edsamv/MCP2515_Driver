# MCP2515 Driver

A driver for the **MCP2515 CAN controller**, with a demo of two nodes communicating over a CAN bus. One node sends dummy motor data, and the other receives it.

## What it does

The project has two nodes:

**Button node (`button_node/`)**
1. Waits for a button press.
2. Sends a CAN message over the bus through the MCP2515.
3. **Toggles an LED** to show that the transfer took place.
4. Sends **dummy motor data** (e.g. [speed, torque, temperature, etc.]) as CAN frames, simulating what a real motor controller would report.

**Receiver (`receiver/`)**
1. Listens on the CAN bus through its own MCP2515.
2. Receives the incoming frames.
3. Decodes the dummy motor data and [prints it / displays it / acts on it].

## How CAN works

CAN (Controller Area Network) is a serial bus designed for reliable communication between devices, widely used in cars, motors, and industrial equipment.

- **Two-wire bus:** All nodes share just two wires, **CANH** and **CANL**. Data is sent as the voltage *difference* between them, which makes it very resistant to noise.
- **Message-based, not address-based:** Frames aren't sent *to* a specific device. Each frame carries an **ID** that describes what the data is (e.g. "motor speed"). Any node can listen and pick the IDs it cares about.
- **Frame structure:** A standard frame contains an ID (11 bits), a length code, up to **8 bytes of data**, and a CRC for error checking.
- **Arbitration:** If two nodes transmit at once, the one with the **lower ID wins** and keeps sending. The other backs off and retries automatically, so no data is lost.
- **Built-in error handling:** CAN detects errors using CRC checks and acknowledgements, and corrupted frames are automatically retransmitted.
- **Shared bitrate:** All nodes must use the same bitrate (e.g. 500 kbps).
- **Termination:** The bus needs a 120 Ω resistor at each end to prevent signal reflections.

## Where the MCP2515 fits in

Most microcontrollers don't have a CAN controller built in. The MCP2515 handles the CAN protocol (framing, arbitration, error checking, buffering) and talks to the microcontroller over **SPI**. A CAN transceiver chip (e.g. TJA1050), usually on the same module, converts its signals into the differential CANH/CANL levels on the bus.

```
MCU  --SPI-->  MCP2515  -->  CAN transceiver  -->  CANH/CANL bus
```

## Project structure

```
MCP2515_Driver/
├── button_node/   # Sends CAN frames on button press, toggles LED, sends dummy motor data
└── receiver/      # Receives and handles the CAN frames
```

## CAN message format

| CAN ID  | Contents         | Data bytes                                |
|---------|------------------|-------------------------------------------|
| [0x...] | Button / trigger | [...]                                     |
| [0x...] | Dummy motor data | [e.g. speed (2B), torque (2B), temp (1B)] |
