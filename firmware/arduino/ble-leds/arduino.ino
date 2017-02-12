#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(240, PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySerial(10, 11); // RX, TX

void setup()
{
  pinMode(13, OUTPUT);

  mySerial.begin(115200);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  rainbow(20);

  colorWipe(0, 10);
}

#define PACKET_SIZE			5
#define MAX_MILLIS_TO_WAIT	1000
unsigned long starttime;
boolean ledState = HIGH;

void loop()
{
  digitalWrite(13, ledState);
  
  ledState = !ledState;
  
  	starttime = millis();

	unsigned char data[PACKET_SIZE] = { 0 };

	while ((mySerial.available() < PACKET_SIZE) && ((millis() - starttime) < MAX_MILLIS_TO_WAIT))
	{
		// hang in this loop until we either get 11 bytes of data or 1 second has gone by
		// Serial.println("Waiting...");
	}

	if (mySerial.available() < PACKET_SIZE)
	{
		// the data didn't come in - handle that problem here
		// Serial.println("ERROR - Didn't get 11 bytes of data!");
	}
	else
	{
		for (int n = 0; n < PACKET_SIZE; n++)
		{
			data[n] = mySerial.read(); // Then: Get them.
		}

		unsigned char crc = data[PACKET_SIZE - 1];

		if (crc != 0xff)
		{
			Serial.print("Invalid data received: ");
			Serial.println(crc, DEC);   
      
			while ((mySerial.available() > 0))
			{
				int endCode = mySerial.read();
        
				if (endCode == 0xff)
				{
					mySerial.println("Got end code");
					return;
				}
				else
				{
					mySerial.print("Waiting for end code: ");
					mySerial.println(endCode, DEC);   
				}
			}
		}

		uint16_t pixel = data[0];
		uint8_t r = data[1];
		uint8_t g = data[2];
		uint8_t b = data[3];

		strip.setPixelColor(pixel, r, g, b);
              
		strip.show();
	}
};









/*

void loop() {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  rainbow(20);
  rainbowCycle(20);
}

*/

void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for(uint16_t j = 0; j < 256; j++)
  {
    for(uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, wheel((i+j) & 255));
    }

    strip.show();

    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

/**
 * Input a value 0 to 255 to get a color value.
 * The colours are a transition r - g - b - back to r.
 */
uint32_t wheel(byte wheelPos)
{
  if (wheelPos < 85)
  {
    return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
  }
  else if (wheelPos < 170)
  {
    wheelPos -= 85;

    return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
  }
  else
  {
    wheelPos -= 170;
    
    return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
}

