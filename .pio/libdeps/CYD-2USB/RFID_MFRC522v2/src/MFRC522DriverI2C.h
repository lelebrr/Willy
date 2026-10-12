/* SPDX-License-Identifier: LGPL-2.1 */
#pragma once

#include <MFRC522Driver.h>
#include <Wire.h>

class MFRC522DriverI2C : public MFRC522Driver {
public:
  // using PCD_Register = MFRC522Constants::PCD_Register;
  /////////////////////////////////////////////////////////////////////////////////////
  // Functions for setting up the Arduino.
  /////////////////////////////////////////////////////////////////////////////////////

  bool init() override;

  /////////////////////////////////////////////////////////////////////////////////////
  // Basic interface functions for communicating with the MFRC522.
  /////////////////////////////////////////////////////////////////////////////////////
  void PCD_WriteRegister(const PCD_Register reg, const byte value) override;
  void PCD_WriteRegister(const PCD_Register reg, const byte count, byte *const values) override;
  byte PCD_ReadRegister(const PCD_Register reg) override;
  void PCD_ReadRegister(
      const PCD_Register reg, const byte count, byte *const values, const byte rxAlign = 0
  ) override;

  MFRC522DriverI2C(const byte slaveAdr = 0x28, int sdaPin = -1, int sclPin = -1, TwoWire &wire = Wire)
      : MFRC522Driver(), _slaveAdr(slaveAdr), _sdaPin(sdaPin), _sclPin(sclPin), _wire(wire) {};

protected:
  // Address of mfrc522.
  const byte _slaveAdr;

  // Wire instance.
  int _sdaPin;
  int _sclPin;
  TwoWire &_wire;
};
