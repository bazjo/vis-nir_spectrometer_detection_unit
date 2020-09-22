#include <SPI.h>
#include <Wire.h>

const int dataReadyPin = 9;
const int CS = 10;


void setup()
{
  Serial.begin(38400);
  Serial.println("Initializing");
  
  setXOFreq();
  Serial.println("XO Frequency set");

  SPI.begin(10);
  //SPI.setClockDivider(10,21);
  //SPI.setDataMode(10, SPI_MODE3);
  //SPI.setBitOrder(MSBFIRST); 

  /*REG_SPI0_WPMR=0x53504900;
  REG_SPI0_CR=0x1;
  SPI0->SPI_CSR[0]=0x00001580;*/ 
  
}
 int val = 0;

void loop()
{
  SPI.beginTransaction();
  SPI.transfer(10,0x42);
  SPI.endTransaction();

  
  Serial.println(val);
  
  delay(50);
}

void setXOFreq(){
  //byte XORegisters[8] = {0x22, 0xDF, 0x1F, 0x59, 0x25, 0x08, 0x50, 0x23}; //614.4 kHz
  //byte Registers[8] = {0x23, 0x73, 0xD0, 0x96, 0x2E, 0x26, 0x64, 0x01}; //7.12 MHz
  byte XORegisters[8] = {0x22, 0x6F, 0xB8, 0x80, 0x24, 0x08, 0x64, 0x33}; //300 kHz

  //Deassert OE bit
  Wire1.begin();
  Wire1.beginTransmission(0x55);
  Wire1.write(0x84); //Register Address
  Wire1.write(0b00000000); //Register Value
  Wire1.endTransmission();

  //Write the new frequency configuration (LP1 and LP2)
  Wire1.begin();
  Wire1.beginTransmission(0x55);
  Wire1.write(0x00);
  Wire1.write(XORegisters[0]);
  Wire1.endTransmission();

  //Write the new frequency configuration (M, HS_DIV, HS_DIV)
  Wire1.beginTransmission(0x55);
  Wire1.write(0x05);
  Wire1.write(&XORegisters[1], 7);
  Wire1.endTransmission();

  //Assert FCAL register bit (This bit is self-clearing) and Assert OE register bit
  Wire1.begin();
  Wire1.beginTransmission(0x55);
  Wire1.write(0x84);
  Wire1.write(0b00000101);
  Wire1.endTransmission();
}
