# Automatic Mains Failure (AMF) System Using Arduino UNO

## Project Overview

The Automatic Mains Failure (AMF) system is an Arduino UNO-based
demonstration prototype that detects the failure of the main power
supply and switches the load to an available backup power source. When
the main supply is restored, the system transfers the load back after a
stabilization delay.

The project demonstrates supply monitoring, relay control, timed
changeover, and **break-before-make switching**.

## Objectives

-   Detect the availability or failure of the main power supply.
-   Monitor the backup generator supply.
-   Control the main and backup-source relays.
-   Introduce a delay during changeover to prevent both sources from
    being connected simultaneously.
-   Indicate the operating condition using LEDs and an LCD.

## Main Components

-   Arduino UNO
-   Two SPDT relay modules
-   Main supply sensing circuit/module
-   Generator supply sensing circuit/module
-   LCD display with I2C interface
-   Red, yellow, and green LEDs
-   Resistors, connecting wires, and suitable power supplies

*Update this list to match the exact components and modules used in your
final hardware setup.*

## Pin Configuration

  Component / Function                         Arduino UNO Pin
  ------------------------------------------ -----------------
  Red LED -- Main supply indication                         D5
  Yellow LED -- Changeover indication                       D6
  Green LED -- Generator supply indication                  D7
  Generator relay (R2)                                      D8
  Main supply relay (R1)                                    D9
  Generator supply sensing input                           D10
  Main supply sensing input                                D11
  Load sensing input (reserved)                            D12

## Relay Configuration

The relay modules used in the prototype are configured as
**active-LOW**:

  Arduino Output   Relay State
  ---------------- -------------
  LOW              Relay ON
  HIGH             Relay OFF

Confirm the input polarity of the actual relay modules before powering
the circuit. Do not assume every relay module uses active-LOW logic.

## Working Principle

1.  **Normal operation:** When the main supply is detected, the Arduino
    keeps the main relay (R1) ON and the generator relay (R2) OFF. The
    main-supply indicator is activated.
2.  **Main supply failure:** When the main supply is no longer detected,
    the Arduino turns OFF R1 and keeps R2 OFF.
3.  **Changeover delay:** The controller waits for the configured delay
    (3 seconds in the current design) while monitoring the supply
    inputs. The yellow LED indicates the changeover period.
4.  **Backup supply operation:** If the main supply remains unavailable
    and the backup supply is detected, the controller activates R2. The
    generator indicator shows that the backup source is supplying the
    load.
5.  **Main supply restoration:** When the main supply returns, the
    controller turns OFF R2, waits for the configured stabilization
    delay, and then reconnects the load to the main supply by activating
    R1, provided the main supply remains available.

The sequence is designed to use **break-before-make switching**: the
active source relay is turned OFF before the other source relay is
turned ON.

## Changeover Timing

  Parameter                                       Current Configuration
  --------------------------------------------- -----------------------
  Main-to-backup changeover delay                             3 seconds
  Main supply restoration stabilization delay                 3 seconds
  Switching method                                    Break-before-make

The timing values should be checked against the final Arduino sketch
before testing or submission.

## Safety and Limitations

-   This is an educational, low-voltage demonstration prototype. It is
    **not an approved industrial or mains-rated AMF panel**.
-   Do not connect the prototype directly to 230 V AC mains unless the
    sensing, isolation, protection, relay ratings, enclosure, wiring,
    and installation have been designed and verified by a qualified
    electrical professional.
-   Use properly isolated, appropriately rated sensing interfaces. Never
    connect mains voltage directly to Arduino pins.
-   The two power sources must never be connected together. Use suitable
    electrical and mechanical interlocking and correctly rated transfer
    equipment for any real installation.
-   The prototype demonstrates the control sequence; it does not by
    itself provide certified protection, generator starting automation,
    or guaranteed uninterrupted power.

## Getting Started

1.  Assemble the low-voltage demonstration circuit according to the
    final circuit diagram.
2.  Check all Arduino pin connections and confirm the relay module's
    active-LOW or active-HIGH behaviour.
3.  Open the project sketch in the Arduino IDE.
4.  Select **Arduino UNO** as the board and choose the correct COM port.
5.  Upload the sketch.
6.  Test the main-supply failure and restoration sequence using safe,
    isolated low-voltage sensing inputs before connecting any load.

## Future Improvements

-   Add generator start/stop control with suitable isolated interfaces.
-   Add fault detection and alarms.
-   Add event logging and an operating-time record.
-   Add IoT-based remote monitoring.
-   Improve electrical isolation, interlocking, and protection for a
    professionally engineered installation.

## Project Status

Educational demonstration prototype. Update this README whenever the
circuit, pin assignments, timing values, or software behaviour changes.
