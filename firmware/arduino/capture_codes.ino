#include <IRremote.h>

const int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);

void setup()
{
  while (!Serial);
  
  Serial.begin(9600);
  
  Serial.println("go!");
  
  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
}

template <typename T> T reverse(T n, size_t b = sizeof(T) * 8)
{
  T rv = 0;

  for (size_t i = 0; i < b; ++i, n >>= 1)
  {
    rv = (rv << 1) | (n & 0x01);
  }

  return rv;
}

void printSIRC(unsigned short value)
{
  Serial.print("SIRC: address: ");
  unsigned short address = reverse((value & 0x1f) << 11);
  Serial.print(address, DEC);

  unsigned short command = reverse((value >> 5) << 9);
  Serial.print(" command: ");
  Serial.println(command, DEC);
}

void printNEC(unsigned int value)
{
/*  Serial.print("NEC: address: ");
  unsigned int address = reverse((value & 0xffff) << 16);
  Serial.print(address, DEC);

  unsigned int command = reverse(value & 0xffff0000);
  Serial.print(" command: ");
  Serial.println(command, DEC);*/
}

void loop()
{
  decode_results results;

  if (irrecv.decode(&results))
  {
    if (results.decode_type == NEC)
    {
      printNEC(results.value);
    }
    else if (results.decode_type == SONY)
    {
      printSIRC(results.value);
    }
    else if (results.decode_type == RC5)
    {
      Serial.print("RC5: ");
    }
    else if (results.decode_type == RC6)
    {
      Serial.print("RC6: ");
    }
    else if (results.decode_type == UNKNOWN)
    {
      Serial.print("UNKNOWN: ");
    }
    
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}

