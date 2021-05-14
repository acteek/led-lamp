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
uint8_t currentPatternNumber = 5; // current pattern index
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
  currentHue++;
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

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  60

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120
bool gReverseDirection = false;


void fire()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

SimplePatternList patterns = {rainbow, cyan, blue, green, yellow, fire};
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

    // Handle remote comand
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
    EVERY_N_MILLISECONDS(40)
    {
      patterns[currentPatternNumber]();
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      FastLED.show();
    }
  }
}
