/* Welcome to DmxMaster. This library allows you to control DMX stage and
** architectural lighting and visual effects easily from Arduino. DmxMaster
** is compatible with the Tinker.it! DMX shield and all known DIY Arduino
** DMX control circuits.
**
** DmxMaster is available from: http://code.google.com/p/tinkerit/
** Help and support: http://groups.google.com/group/DmxMaster */

/* To use DmxMaster, you will need the following line. Arduino will
** auto-insert it if you select Sketch > Import Library > DmxMaster. */

#include <DmxMaster.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

//#define DEBUG


const int ledPin = 13;
const int lightStartIDs[] = {1, 9, 17, 25};
const int numlights = sizeof(lightStartIDs) / sizeof(int);
int lightsequence[numlights];
int lightstep[numlights];
int lightsequencecount[numlights];
const int repeatsequence = 3;

uint32_t flashSequenceBuffer[100];

#define STOPCODE 0xF1F1F1F1

#define BRIGHT 255
#define RED    0x00FF0000
#define GREEN  0x0000FF00
#define BLUE   0x000000FF
#define WHITE  0xFF000000
#define YELLOW 0x00FFFF00
#define PINK   0x00FF00FF
#define CYAN   0x0000FFFF
#define OFF    0x00000000

const byte buffsize = 75;

const uint32_t fseq00[] PROGMEM = {
  WHITE, WHITE, OFF,
  WHITE, WHITE, OFF,
  WHITE, WHITE, OFF,
  WHITE, WHITE, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  STOPCODE
};

const uint32_t fseq01[] PROGMEM = {
  WHITE, WHITE, WHITE, WHITE, WHITE,
  WHITE, WHITE, WHITE, WHITE, OFF,
  RED, RED, RED, RED, RED,
  RED, RED, RED, RED, OFF,
  STOPCODE
};

const uint32_t fseq02[] PROGMEM = {
  WHITE, WHITE, WHITE,
  RED, RED, RED,
  WHITE, WHITE, WHITE,
  RED, RED, RED,
  WHITE, WHITE, WHITE,
  RED, RED, RED,
  WHITE, WHITE, WHITE,
  RED, RED, RED,
  STOPCODE
};

const uint32_t fseq10[] PROGMEM = {
  BLUE, BLUE, OFF,
  BLUE, BLUE, OFF,
  BLUE, BLUE, OFF,
  BLUE, BLUE, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  RED, RED, OFF,
  STOPCODE
};

const uint32_t fseq11[] PROGMEM = {
  PINK, PINK, OFF,
  PINK, PINK, OFF,
  PINK, PINK, OFF,
  PINK, PINK, OFF,
  STOPCODE
};

const uint32_t fseq12[] PROGMEM = {
  BLUE, BLUE, BLUE,
  RED, RED, RED,
  BLUE, BLUE, BLUE,
  RED, RED, RED,
  BLUE, BLUE, BLUE,
  RED, RED, RED,
  BLUE, BLUE, BLUE,
  RED, RED, RED,
  STOPCODE
};

const uint32_t fseq13[] PROGMEM = {
  RED, RED, RED, RED, RED,
  RED, RED, RED, RED, OFF,
  BLUE, BLUE, BLUE, BLUE, BLUE,
  BLUE, BLUE, BLUE, BLUE, OFF,
  STOPCODE
};

const uint32_t fseq20[] PROGMEM = {
  GREEN, GREEN, OFF,
  GREEN, GREEN, OFF,
  GREEN, GREEN, OFF,
  GREEN, GREEN, OFF,
  YELLOW, YELLOW, OFF,
  YELLOW, YELLOW, OFF,
  YELLOW, YELLOW, OFF,
  YELLOW, YELLOW, OFF,
  STOPCODE
};

const uint32_t fseq21[] PROGMEM = {
  GREEN, GREEN, GREEN, GREEN, GREEN,
  GREEN, GREEN, GREEN, GREEN, OFF,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,
  YELLOW, YELLOW, YELLOW, YELLOW, OFF,
  STOPCODE
};


const uint32_t fseq30[] PROGMEM = {
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  STOPCODE
};

const uint32_t fseq31[] PROGMEM = {
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  WHITE, WHITE, WHITE, OFF, OFF,
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  WHITE, WHITE, WHITE, OFF, OFF,
  YELLOW, YELLOW, YELLOW, OFF, OFF,
  WHITE, WHITE, WHITE, OFF, OFF,
  STOPCODE
};

const uint32_t fseq32[] PROGMEM = {
  YELLOW, YELLOW, OFF, OFF, YELLOW, YELLOW, OFF, OFF, OFF, OFF,
  YELLOW, YELLOW, OFF, OFF, YELLOW, YELLOW, OFF, OFF, OFF, OFF,
  YELLOW, YELLOW, OFF, OFF, YELLOW, YELLOW, OFF, OFF, OFF, OFF,
  STOPCODE
};

const uint32_t fseq40[] PROGMEM = {
  RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, 
  GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, 
  CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, 
  BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, 
  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
  PINK, PINK, PINK, PINK, PINK, PINK, PINK, PINK, PINK, PINK, 
  STOPCODE
};

//const uint32_t * const flashSequence1[] PROGMEM = {fseq11, fseq12};
//const uint32_t * const flashSequence2[] PROGMEM = {fseq21, fseq22};
//const uint32_t * const flashSequence3[] PROGMEM = {fseq31};
const uint32_t * const metaSequences[][5] = {
  {fseq00, fseq01, fseq00, fseq02, fseq00},
  {fseq10, fseq11, fseq10, fseq12, fseq13},
  {fseq20, fseq21, fseq20, fseq21, fseq20},
  {fseq30, fseq31, fseq30, fseq31, fseq32},
  {fseq40, fseq40, fseq40, fseq40, fseq40}
};

const int metaSeqCnt = 5; // HARDCODED - Edit when changing metasequences
word flashSeqID = 0;  // will be changed by code at boot
const word flashSeqCnt = 5; // HARDCODED - Edit when changing metasequences

uint32_t flashSequenceAddrs[10];
byte flashSeqLen[10];

const word flashaddr = 0;

void setup() {
  int l;

  /* The most common pin for DMX output is pin 3, which DmxMaster
  ** uses by default. If you need to change that, do it here. */
  DmxMaster.usePin(3);

  /* DMX devices typically need to receive a complete set of channels
  ** even if you only need to adjust the first channel. You can
  ** easily change the number of channels sent here. If you don't
  ** do this, DmxMaster will set the maximum channel number to the
  ** highest channel you DmxMaster.write() to. */
  DmxMaster.maxChannel(numlights * 8);

  pinMode(ledPin, OUTPUT);

  // Read EEPROM for last used meta sequence, and increment it
  flashSeqID = EEPROM.read(flashaddr) + 1;
  if (flashSeqID >= metaSeqCnt) {
    flashSeqID = 0;
  }
  EEPROM.write(flashaddr, flashSeqID);

  // Calculate current sequence stats
  word seqaddr;
  byte len;
  uint32_t seqbuffer[buffsize];
  for (l = 0; l < flashSeqCnt; l++) {
    seqaddr = (uint32_t)metaSequences[flashSeqID][l];
    flashSequenceAddrs[l] = seqaddr;
    memcpy_P(&seqbuffer, (const void *)seqaddr, (buffsize - 1) * sizeof(uint32_t));
    for (len = 0; len < buffsize; len ++) {
      if (seqbuffer[len] == STOPCODE) {
        break;
      }
    }
    flashSeqLen[l] = len;
  }





#ifdef DEBUG
  Serial.begin(115200);
  Serial.println();
  Serial.println("WELCOME MASTER!");
  Serial.print("Number of lights = ");
  Serial.println(numlights);
  Serial.print("Number of Meta Sequences = ");
  Serial.println(metaSeqCnt);
  Serial.print("Current Meta Sequence = ");
  Serial.println(flashSeqID);

  Serial.println();
  Serial.print("Seq00Addr = ");
  Serial.println((uint32_t)fseq00, HEX);
  Serial.print("Seq01Addr = ");
  Serial.println((uint32_t)fseq01, HEX);
  Serial.print("Seq10Addr = ");
  Serial.println((uint32_t)fseq10, HEX);
  Serial.print("Seq11Addr = ");
  Serial.println((uint32_t)fseq11, HEX);
  Serial.print("Seq20Addr = ");
  Serial.println((uint32_t)fseq20, HEX);
  Serial.println();

  for (l = 0; l < metaSeqCnt; l++) {
    Serial.print((uint32_t)metaSequences[l][0], HEX);
    Serial.print(", ");
    Serial.println((uint32_t)metaSequences[l][1], HEX);
  }

  for (l = 0; l < flashSeqCnt; l++) {
    Serial.print("Address [");
    Serial.print(l, HEX);
    Serial.print("] = ");
    Serial.print(flashSequenceAddrs[l], HEX);
    Serial.print("; Len = ");
    Serial.println(flashSeqLen[l]);
  }

#endif

  for (l = 0; l < numlights; l++) {
    lightsequence[l] = random(flashSeqCnt);
    lightstep[l] = random(flashSeqLen[lightsequence[l]]);
    lightsequencecount[l] = random(repeatsequence);
#ifdef DEBUG
    Serial.print("Random SequenceID for light ");
    Serial.print(l);
    Serial.print(" is ");
    Serial.print(lightsequence[l]);
    Serial.print(" at step #");
    Serial.println(lightstep[l]);
#endif
  }

#ifdef DEBUG

  word addr;
  uint32_t val;
  for (l = 0; l < flashSeqCnt; l++) {
    Serial.print("Seq[");
    Serial.print(l);
    Serial.println("]");
    for (len = 0; len < flashSeqLen[l]; len ++) {
      addr = flashSequenceAddrs[l] + len * sizeof(uint32_t);
      val = pgm_read_dword(addr);
      Serial.print("Address = ");
      Serial.print(addr, HEX);
      Serial.print("; value = ");
      Serial.println(val, HEX);
    }
  }

#endif

}



void loop() {
  int x;
  int l;
  uint32_t color;
  int coloridx;
  word addr;

  for (l = 0; l < numlights; l++) {
    // writeLight(flashSequence[coloridx], lightStartIDs[l]);
    lightstep[l] += 1;
    if (lightstep[l] >= flashSeqLen[lightsequence[l]]) {
      // Need to choose a new sequence
      lightsequencecount[l] += 1;
      if (lightsequencecount[l] >= repeatsequence){
        lightsequencecount[l] = 0;
        lightsequence[l] = random(flashSeqCnt);
      }
      lightstep[l] = 0;
    }
    addr = flashSequenceAddrs[lightsequence[l]] + lightstep[l] * sizeof(uint32_t);
    color = pgm_read_dword(addr);
#ifdef DEBUG
    Serial.print("Light [");
    Serial.print(l);
    Serial.print("] Sequence [");
    Serial.print(lightsequence[l]);
    Serial.print("] Step [");
    Serial.print(lightstep[l]);
    Serial.print("] Color [");
    Serial.print(color,HEX);
    Serial.println("]");
#endif
    writeLight(color, lightStartIDs[l]);
  }
  delay(50);
  digitalWrite(ledPin, !digitalRead(ledPin));
}

// Takes in a sequence(x), and a light id and returns a reordered x based on that light's sequence offset.
//int reorder(int x, int l) {
//  int num;
//  num = x + lightsequence[l];
//  if (num >= flashseq) {
//    return abs(flashseq - num);
//  } else {
//    return num;
//  }
//}

void writeLight(uint32_t color, uint8_t lightstartid) {
  DmxMaster.write(lightstartid, BRIGHT);
  DmxMaster.write(lightstartid + 1, Red(color));
  DmxMaster.write(lightstartid + 2, Green(color));
  DmxMaster.write(lightstartid + 3, Blue(color));
  DmxMaster.write(lightstartid + 4, White(color));
}

uint8_t White(uint32_t color)
{
  return (color >> 24) & 0xFF;
}

uint8_t Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
  return color & 0xFF;
}

uint32_t Color(uint8_t w, uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}



