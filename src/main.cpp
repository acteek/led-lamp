#include <Arduino.h>
#include <FastLED.h>
#include <SoftwareSerial.h>

//Setup
#define RX_PIN 10
#define TX_PIN 11
#define LED_PIN 2
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 85
#define BRIGHTNESS 100
#define FRAMES_PER_SECOND 120

uint8_t currentHue = 0;                  // color HUE value
uint8_t currentPatternNumber = 0;        // current pattern index
boolean isLedOn = true;                  // lighting led flag
SoftwareSerial BTSerial(RX_PIN, TX_PIN); // init BT UART port
CRGB leds[NUM_LEDS];
typedef void (*SimplePatternList[])();

void setup()
{

  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  BTSerial.begin(9600); // 38400 for setting mode
  BTSerial.setTimeout(1000);
  delay(2000); // wait for init UART

  // init led strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.println("Led has been inited...");
}

//--- Start describe patterns

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, currentHue, 7);
}

SimplePatternList patterns = {rainbow};
//--- Stop describe patterns

void loop()
{

  if (BTSerial.available())
  {

    int command = BTSerial.parseInt();

    String message = "Recieve msg: ";
    message += command;
    Serial.println(message);

    switch (command)
    {
    case 99:
      isLedOn = !isLedOn;
      break;
    default:
      if (command < sizeof(patterns))
      {
        currentPatternNumber = command;
      }
      break;
    }
  }

  if (isLedOn)
  {
    // Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternNumber]();  // delay(300); // delay to parse ir signal

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);   // insert a delay to keep the framerate modest
    EVERY_N_MILLISECONDS(20) { currentHue++; } // slowly cycle the "base color" through the rainbow
  }

}
