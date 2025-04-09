# Novatek nt36672c Controller Driver for Windows
In this repository you can find the nt36672c controller driver for Windows (KMDF).
This driver only works with nt36672c controller found in POCO X3 Pro.

This repository has been forked from https://github.com/theR4K/SynapticsTouch for the base and modified to work with Focal Tech digitizers, and again it was forked from https://github.com/gus33000/FocalTechTouch and modified to work with NovaTek Microelectronics digitizers.

## Status
| Feature                        | Notes                                                                             | Status         |
|--------------------------------|-----------------------------------------------------------------------------------|----------------|
| Touch                          | Requires touch precision tunning, finger detection isn't as accurate as it can be | ✅            |
| Suspend/Resume                 | Requires a proper Suspend/Resume sequence. Currently it's just workaround in ACPI | ✅            |
| Wakeup Gesture                 | Most likely possible to get working, needs further testing                        | ⚠️           |
| Multi-Touch                    |                                                                                   | ✅            |
| Tianma/CSOT Panel detection    |                                                                                   | ✅            |
| Palm detection                 | Might require Xiaomi Touch features                                               | ❔            |
| Xiaomi Touch features          | Can be implemented, but are they really needed?                                   | ❔            |
