#include <Wire.h>

void setup()
{
  byte Registers[8] = {0x22, 0x6F, 0xB8, 0x80, 0x24, 0x08, 0x64, 0x33}; //300 kHz
  //byte Registers[8] = {0x23, 0x73, 0xD0, 0x96, 0x2E, 0x26, 0x64, 0x01}; //7.12 MHz

  //Deassert OE bit
  Wire.begin();
  Wire.beginTransmission(0x55);
  Wire.write(0x84); //Register Address
  Wire.write(0b00000000); //Register Value
  Wire.endTransmission();

  //Write the new frequency configuration (LP1 and LP2)
  Wire.begin();
  Wire.beginTransmission(0x55);
  Wire.write(0x00);
  Wire.write(Registers[0]);
  Wire.endTransmission();

  //Write the new frequency configuration (M, HS_DIV, HS_DIV)
  Wire.beginTransmission(0x55);
  Wire.write(0x05);
  Wire.write(&Registers[1], 7);
  Wire.endTransmission();

  //Assert FCAL register bit (This bit is self-clearing) and Assert OE register bit
  Wire.begin();
  Wire.beginTransmission(0x55);
  Wire.write(0x84);
  Wire.write(0b00000101);
  Wire.endTransmission();
}


void loop()
{

}
