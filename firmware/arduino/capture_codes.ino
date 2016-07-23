// use https://github.com/z3t0/Arduino-IRremote/releases library
// connect TSOP382 IR photo sensor output pin to arduino pin 12 (connect also connect GND and VCC to arduino)

#include <IRremote.h>

int recvPin = 11;
IRrecv irrecv(recvPin);

void  setup ( )
{
  Serial.begin(115200);
  irrecv.enableIRIn();

  Serial.println("go3!");
};

unsigned char reverseBits(unsigned char x)
{
  x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
  x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
  x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);

  return x;    
};

void printCode(decode_results* results)
{
  unsigned long d = results->value;

  switch (results->decode_type)
  {
    case SONY:
      {
        unsigned char sircAddress = reverseBits((d & 0b11111) << 3);
        unsigned char sircCode = reverseBits( ((unsigned char)((d & 0b11111111100000) >> 5)) << 1);
        Serial.print("SIRC address: ");
        Serial.print(sircAddress, DEC);
        Serial.print(", command: ");
        Serial.println(sircCode);
      }
      break;
    case RC5:
      {
        unsigned int rc5Command = d & 0b111111;
        unsigned int rc5Address = ((d & 0b11111000000) >> 6);
        Serial.print("RC5 address: ");
        Serial.print(rc5Address, DEC);
        Serial.print(", command: ");
        Serial.println(rc5Command, DEC);
      }
      break;
    case NEC:
      {
        unsigned char necCommand = reverseBits((d & 0xff00) >> 8);
        unsigned char necCommandInverted = ~reverseBits(d & 0xff);
        unsigned int address = d >> 16;
        unsigned char necAddress = reverseBits((address & 0xff00) >> 8);
        unsigned char necAddressInverted = ~reverseBits(address & 0xff);

        if (necAddress == necAddressInverted)
        {
          address = necAddress;
        }
        else
        {
          address = (reverseBits(address) << 8) | (reverseBits(address >> 8));
        }
        
        Serial.print("NEC address: ");
        Serial.print(address, DEC);
        Serial.print(" (0x");
        Serial.print(address, HEX);
        Serial.print("), command: ");
        Serial.println(necCommand, DEC);
      }
      break;
    default:
      break;
  }
};

void loop ()
{
  decode_results  results;

  if (irrecv.decode(&results))
  {
    printCode(&results);
    Serial.println("");
    irrecv.resume();
  }
};

