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
#define FRAMES_PER_SECOND 120

uint8_t currentHue = 0;           // color HUE value
uint8_t currentPatternNumber = 0; // current pattern index
uint8_t brightness = 100;
boolean isLedOn = true;                  // lighting led flag
SoftwareSerial BTSerial(RX_PIN, TX_PIN); // init BT UART port
CRGB leds[NUM_LEDS];
typedef void (*SimplePatternList[])();

void setup()
{

  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  BTSerial.begin(9600); // 38400 for setting mode
  BTSerial.setTimeout(10);

  // init led strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);

  Serial.println("Led has been inited...");
}

//--- Start describe patterns

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, currentHue, 7);
}

void cyan()
{
  for (int i = NUM_LEDS - 1; i > 0; i--)
  {
    leds[i] = CRGB::Cyan; //CHSV(127, 255, 255);
  }
}

void blue()
{
  for (int i = NUM_LEDS - 1; i > 0; i--)
  {
    leds[i] = CHSV(160, 255, 255);
    ;
  }
}

void green()
{
  for (int i = NUM_LEDS - 1; i > 0; i--)
  {
    leds[i] = CRGB::Green;
  }
}

void yellow()
{
  for (int i = NUM_LEDS - 1; i > 0; i--)
  {
    leds[i] = CRGB::Yellow;
    ;
  }
}

SimplePatternList patterns = {rainbow, cyan, blue, green, yellow};
//--- Stop describe patterns

void loop()
{
  // Parse comand
  if (BTSerial.available())
  {

    char buf[50];
    int num = BTSerial.readBytesUntil(';', buf, 50);
    buf[num] = NULL;

    int comand[10];
    int count = 0;
    char *offset = buf;
    while (true)
    {
      comand[count++] = atoi(offset);
      offset = strchr(offset, ',');
      if (offset)
        offset++;
      else
        break;
    }

    // Handle comand
    switch (comand[0])
    {
    case 1: // On/Off leds
      FastLED.clear(true);
      isLedOn = !isLedOn;
      delay(500);
      if (isLedOn)
        Serial.println("Led on ...");
      else
        Serial.println("Led off ...");
      break;
    case 3: // Set brightness 0-255
      brightness = comand[1];
      FastLED.setBrightness(brightness);
      FastLED.show();
      Serial.print("Set brightness: ");
      Serial.println(brightness);
      break;
    case 5: // Set Pattern
      currentPatternNumber = comand[1];
      Serial.print("Set pattern to: ");
      Serial.println(currentPatternNumber);
      break;
    default:
      Serial.print("Unknown command: ");
      for (int i = 0; i < count; i++)
        Serial.println(comand[i]);
      break;
    }
  }

  // Periodic update, needs for live patterns
  if (isLedOn)
  {
    EVERY_N_MILLISECONDS(90)
    {
      patterns[currentPatternNumber]();
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      FastLED.show();
      currentHue++;
      FastLED.show();
    }
  }
}
