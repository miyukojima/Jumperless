// SPDX-License-Identifier: MIT

#include "CH446Q.h"
#include "MatrixStateRP2040.h"
#include "NetsToChipConnections.h"
#include "LEDs.h"
#include "Peripherals.h"
#include "JumperlessDefinesRP2040.h"

#include "hardware/pio.h"

#include "spi.pio.h"
#include "pio_spi.h"

#define MYNAMEISERIC 0 // on the board I sent to eric, the data and clock lines are bodged to GPIO 18 and 19. To allow for using hardware SPI

int chipToPinArray[12] = {CS_A, CS_B, CS_C, CS_D, CS_E, CS_F, CS_G, CS_H, CS_I, CS_J, CS_K, CS_L};
PIO pio = pio0;

uint sm = pio_claim_unused_sm(pio, true);

volatile int chipSelect = 0;
volatile uint32_t irq_flags = 0;

void isrFromPio(void)
{
  switch (chipSelect)
  {
  case CHIP_A:
  {
    digitalWriteFast(CS_A, HIGH);
    break;
  }
  case CHIP_B:
  {
    digitalWriteFast(CS_B, HIGH);
    break;
  }
  case CHIP_C:
  {
    digitalWriteFast(CS_C, HIGH);
    break;
  }
  case CHIP_D:
  {
    digitalWriteFast(CS_D, HIGH);
    break;
  }
  case CHIP_E:
  {
    digitalWriteFast(CS_E, HIGH);
    break;
  }
  case CHIP_F:
  {
    digitalWriteFast(CS_F, HIGH);
    break;
  }
  case CHIP_G:
  {
    digitalWriteFast(CS_G, HIGH);
    break;
  }
  case CHIP_H:
  {
    digitalWriteFast(CS_H, HIGH);
    break;
  }
  case CHIP_I:
  {
    digitalWriteFast(CS_I, HIGH);
    break;
  }
  case CHIP_J:
  {
    digitalWriteFast(CS_J, HIGH);
    break;
  }
  case CHIP_K:
  {
    digitalWriteFast(CS_K, HIGH);
    break;
  }
  case CHIP_L:
  {
    digitalWriteFast(CS_L, HIGH);
    break;
  }
  }

  delayMicroseconds(2);
  digitalWriteFast(CS_A, LOW);
  digitalWriteFast(CS_B, LOW);
  digitalWriteFast(CS_C, LOW);
  digitalWriteFast(CS_D, LOW);
  digitalWriteFast(CS_E, LOW);
  digitalWriteFast(CS_F, LOW);
  digitalWriteFast(CS_G, LOW);

  digitalWriteFast(CS_H, LOW);
  digitalWriteFast(CS_I, LOW);
  digitalWriteFast(CS_J, LOW);
  digitalWriteFast(CS_K, LOW);
  digitalWriteFast(CS_L, LOW);
  delayMicroseconds(1);
  irq_flags = pio0_hw->irq;
  pio_interrupt_clear(pio, PIO0_IRQ_0);
  hw_clear_bits(&pio0_hw->irq, irq_flags);
}

void initCH446Q(void)
{

  uint dat = 14;
  uint clk = 15;

  if (MYNAMEISERIC)
  {
    dat = 18;
    clk = 19;
  }

  uint cs = 7;

  irq_add_shared_handler(PIO0_IRQ_0, isrFromPio, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(PIO0_IRQ_0, true);

  uint offset = pio_add_program(pio, &spi_ch446_multi_cs_program);
  // uint offsetCS = pio_add_program(pio, &spi_ch446_cs_handler_program);

  Serial.print("offset: ");
  Serial.println(offset);

  pio_spi_ch446_multi_cs_init(pio, sm, offset, 8, 16, 0, 1, clk, dat);
  // pio_spi_ch446_cs_handler_init(pio, smCS, offsetCS, 256, 1, 8, 20, 6);
  // pinMode(CS_A, OUTPUT);
  // digitalWrite(CS_A, HIGH);

  pinMode(CS_A, OUTPUT);
  pinMode(CS_B, OUTPUT);
  pinMode(CS_C, OUTPUT);
  pinMode(CS_D, OUTPUT);
  pinMode(CS_E, OUTPUT);
  pinMode(CS_F, OUTPUT);
  pinMode(CS_G, OUTPUT);
  pinMode(CS_H, OUTPUT);
  pinMode(CS_I, OUTPUT);
  pinMode(CS_J, OUTPUT);
  pinMode(CS_K, OUTPUT);
  pinMode(CS_L, OUTPUT);

  digitalWrite(CS_A, LOW);
  digitalWrite(CS_B, LOW);
  digitalWrite(CS_C, LOW);
  digitalWrite(CS_D, LOW);
  digitalWrite(CS_E, LOW);
  digitalWrite(CS_F, LOW);
  digitalWrite(CS_G, LOW);
  digitalWrite(CS_H, LOW);
  digitalWrite(CS_I, LOW);
  digitalWrite(CS_J, LOW);
  digitalWrite(CS_K, LOW);
  digitalWrite(CS_L, LOW);

  pinMode(RESETPIN, OUTPUT);

  digitalWrite(RESETPIN, HIGH);
  delay(2);
  digitalWrite(RESETPIN, LOW);
}

void resetArduino(void)
{
  int lastPath = MAX_BRIDGES - 1;
  path[lastPath].chip[0] = CHIP_I;
  path[lastPath].chip[1] = CHIP_I;
  path[lastPath].x[0] = 11;
  path[lastPath].y[0] = 0;
  path[lastPath].x[1] = 15;
  path[lastPath].y[1] = 0;

  sendPath(lastPath, 1);
  delay(15);
  sendPath(lastPath, 0);
}
void sendAllPaths(void) // should we sort them by chip? for now, no
{

  for (int i = 0; i < numberOfPaths; i++)
  {

    if (path[i].skip == true)
    {
      continue;
    }
    sendPath(i, 1);
    if (debugNTCC)
    {
      Serial.print("path ");
      Serial.print(i);
      Serial.print(" \t");
      printPathType(i);
      Serial.print(" \n\r");
      for (int j = 0; j < 4; j++)
      {
        printChipNumToChar(path[i].chip[j]);
        Serial.print("  x[");
        Serial.print(j);
        Serial.print("]:");
        Serial.print(path[i].x[j]);
        Serial.print("   y[");
        Serial.print(j);
        Serial.print("]:");
        Serial.print(path[i].y[j]);
        Serial.print(" \t ");
      }
      Serial.print("\n\n\r");
    }
  }
}

void sendXYraw(int chip, int x, int y, int setOrClear)
{
  uint32_t chAddress = 0;
  chipSelect = chip;

  int chYdata = y;
  int chXdata = x;

  chYdata = chYdata << 5;
  chYdata = chYdata & 0b11100000;

  chXdata = chXdata << 1;
  chXdata = chXdata & 0b00011110;

  chAddress = chYdata | chXdata;

  if (setOrClear == 1)
  {
    chAddress = chAddress | 0b00000001; // this last bit determines whether we set or unset the path
  }

  chAddress = chAddress << 24;

  delayMicroseconds(20);

  pio_sm_put(pio, sm, chAddress);

  delayMicroseconds(40);
}

int probeHalfPeriodus = 10;

int readFloatingOrState(int pin, int rowBeingScanned)
{

  enum measuredState state = floating;
  int readingPullup = 0;
  int readingPullup2 = 0;
  int readingPullup3 = 0;

  int readingPulldown = 0;
  int readingPulldown2 = 0;
  int readingPulldown3 = 0;

  pinMode(pin, INPUT_PULLUP);

  delayMicroseconds(100);

  readingPullup = digitalRead(pin);
  delayMicroseconds(probeHalfPeriodus);
  readingPullup2 = digitalRead(pin);
  delayMicroseconds(probeHalfPeriodus / 2);
  readingPullup3 = digitalRead(pin);

  pinMode(pin, INPUT_PULLDOWN);
  delayMicroseconds(100);

  readingPulldown = digitalRead(pin);
  delayMicroseconds(probeHalfPeriodus);
  readingPulldown2 = digitalRead(pin);
  delayMicroseconds(probeHalfPeriodus / 2);
  readingPulldown3 = digitalRead(pin);

  if (readingPullup != readingPullup2 || readingPullup2 != readingPullup3 && rowBeingScanned != -1)
  {
    if (readingPulldown != readingPulldown2 || readingPulldown2 != readingPulldown3)
    {
      state = probe;

      // leds.setPixelColor(nodesToPixelMap[rowBeingScanned], rainbowList[rainbowIndex][0], rainbowList[rainbowIndex][1], rainbowList[rainbowIndex][2]);

      // Serial.print("probe");
      //
    }
  }
  else
  {

    // Serial.print(readingPulldown);
    // Serial.print("\t");
    if (readingPullup == 1 && readingPulldown == 0)
    {
      // Serial.print("floating");
      // leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 0, 0, 3);
      state = floating;
    }
    else if (readingPullup == 1 && readingPulldown == 1)
    {
      // Serial.print("HIGH");
      // leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 45, 0, 0);
      state = high;
    }
    else if (readingPullup == 0 && readingPulldown == 0)
    {
      // Serial.print("LOW");
      // leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 0, 45, 0);
      state = low;
    }
    else if (readingPullup == 0 && readingPulldown == 1)
    {
      // Serial.print("shorted");
    }
  }
  // Serial.print("\n");
  leds.show();
  // delayMicroseconds(100);

  return state;
}

void startProbe(int probeSpeed)
{
  probeHalfPeriodus = 1000000 / probeSpeed / 2;
  pinMode(19, OUTPUT);
  analogWriteFreq(probeSpeed);
  analogWrite(19, 128);
  // delayMicroseconds(10);
  pinMode(18, INPUT);
}

int rainbowList[12][3] = {
    {45, 35, 8},
    {10, 45, 30},
    {30, 15, 45},
    {8, 27, 45},
    {45, 18, 19},
    {35, 42, 5},
    {02, 45, 35},
    {18, 25, 45},
    {40, 12, 45},
    {10, 32, 45},
    {18, 5, 43},
    {45, 28, 13}};
int rainbowIndex = 0;
int lastFound[5] = {-1, -1, -1, -1, -1};
int nextIsSupply = 0;
int nextIsGnd = 0;
int justCleared = 1;

int scanRows(int pin, bool clearLastFound)
{

  int found = -1;

  if (clearLastFound)
  {
    // for (int i = 0; i < 5; i++)
    // {
    //   lastFound[i] = -1;
    // }


    rainbowIndex++;
    if (rainbowIndex > 11)
    {
      rainbowIndex = 0;
    }

    justCleared = 1;
    nextIsGnd = 0;
    nextIsSupply = 0;
    return -1;
  }

  digitalWrite(RESETPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(RESETPIN, LOW);
  startProbe();
  int chipToConnect = 0;
  int rowBeingScanned = 0;

  int xMapRead = 15;

  if (pin == ADC0_PIN)
  {
    xMapRead = 2;
  }
  else if (pin == ADC1_PIN)
  {
    xMapRead = 3;
  }
  else if (pin == ADC2_PIN)
  {
    xMapRead = 4;
  }
  else if (pin == ADC3_PIN)
  {
    xMapRead = 5;
  }

  // Serial.print("xMapRead: ");
  // Serial.println(xMapRead);

  pinMode(pin, INPUT);
  for (int chipScan = CHIP_A; chipScan < 8; chipScan++)
  {

    sendXYraw(CHIP_L, xMapRead, chipScan, 1);

    for (int yToScan = 1; yToScan < 8; yToScan++)
    {

      sendXYraw(chipScan, 0, 0, 1);
      sendXYraw(chipScan, 0, yToScan, 1);

      // analogRead(ADC0_PIN);

      rowBeingScanned = ch[chipScan].yMap[yToScan];
      if (readFloatingOrState(pin, rowBeingScanned) == probe && rowBeingScanned != lastFound[0])
      {
        found = rowBeingScanned;
        if (nextIsSupply)
        {
          leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 65, 10, 10);
        }
        else if (nextIsGnd)
        {
          leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 10, 65, 10);
        }
        else
        {
          leds.setPixelColor(nodesToPixelMap[rowBeingScanned], rainbowList[rainbowIndex][0], rainbowList[rainbowIndex][1], rainbowList[rainbowIndex][2]);
        }

        leds.show();
        for (int i = 4; i > 0; i--)
        {
          lastFound[i] = lastFound[i - 1];
        }
        lastFound[0] = found;
      }

      sendXYraw(chipScan, 0, 0, 0);
      sendXYraw(chipScan, 0, yToScan, 0);

      if (found != -1)
      {
        break;
      }
    }
    sendXYraw(CHIP_L, 2, chipScan, 0);


  }

  int corners[4] = {1, 30, 31, 60};
  sendXYraw(CHIP_L, xMapRead, 0, 1);
  for (int cornerScan = 0; cornerScan < 4; cornerScan++)
  {

    sendXYraw(CHIP_L, cornerScan + 8, 0, 1);

    // analogRead(ADC0_PIN);

    rowBeingScanned = corners[cornerScan];
    if (readFloatingOrState(pin, rowBeingScanned) == probe && rowBeingScanned != lastFound[0])
    {
      found = rowBeingScanned;
      if (nextIsSupply)
      {
        leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 65, 10, 10);
      }
      else if (nextIsGnd)
      {
        leds.setPixelColor(nodesToPixelMap[rowBeingScanned], 10, 65, 10);
      }
      else
      {
      leds.setPixelColor(nodesToPixelMap[rowBeingScanned], rainbowList[rainbowIndex][0], rainbowList[rainbowIndex][1], rainbowList[rainbowIndex][2]);
      }
      leds.show();
      for (int i = 4; i > 0; i--)
      {
        lastFound[i] = lastFound[i - 1];
      }
      lastFound[0] = found;
    }

    sendXYraw(CHIP_L, cornerScan + 8, 0, 0);

    if (found != -1)
    {
      break;
    }
  }
  sendXYraw(CHIP_L, xMapRead, 0, 0);

  int gp18read = readFloatingOrState(18, -1);

  if (gp18read == probe)
  {
    delayMicroseconds(1000);
    if (readFloatingOrState(18, -1) == probe)
    {
      return -18;
    }
    
  }

  pinMode(19, INPUT);
  delayMicroseconds(900);
  int probeRead = readFloatingOrState(19, -1);

  if (probeRead == high && ((lastFound[0] != SUPPLY_3V3 )))
  {
    found = SUPPLY_3V3;
    if (justCleared)
    {
      nextIsSupply = 1;
      //justCleared = 0;
    }
    else
    {
      leds.setPixelColor(nodesToPixelMap[lastFound[0]], 65, 10, 10);
      nextIsSupply = 0;
    }

    for (int i = 4; i > 0; i--)
    {
      lastFound[i] = lastFound[i - 1];
    }
    lastFound[0] = found;
  }
  else if (probeRead == low && ((lastFound[0] != GND )))
  {
    found = GND;
    if (justCleared)
    {
      // leds.setPixelColor(nodesToPixelMap[lastFound[0]], 0, 0, 0);
      nextIsGnd = 1;
      //justCleared = 0;
    }
    else
    {
      leds.setPixelColor(nodesToPixelMap[lastFound[0]], 10, 65, 10);
      nextIsGnd = 0;
    }

    for (int i = 4; i > 0; i--)
    {
      lastFound[i] = lastFound[i - 1];
    }
    lastFound[0] = found;
  }

  if (justCleared && found != -1)
  {
// Serial.print("\n\rjustCleared: ");
// Serial.println(justCleared);
// Serial.print("nextIsSupply: ");
// Serial.println(nextIsSupply);
// Serial.print("nextIsGnd: ");
// Serial.println(nextIsGnd);

    justCleared = 0;
  }

  return found;

  // return 0;
}

void sendPath(int i, int setOrClear)
{

  uint32_t chAddress = 0;

  int chipToConnect = 0;
  int chYdata = 0;
  int chXdata = 0;

  for (int chip = 0; chip < 4; chip++)
  {
    if (path[i].chip[chip] != -1)
    {
      chipSelect = path[i].chip[chip];

      chipToConnect = path[i].chip[chip];

      if (path[i].y[chip] == -1 || path[i].x[chip] == -1)
      {
        if (debugNTCC)
          Serial.print("!");

        continue;
      }

      chYdata = path[i].y[chip];
      chXdata = path[i].x[chip];

      chYdata = chYdata << 5;
      chYdata = chYdata & 0b11100000;

      chXdata = chXdata << 1;
      chXdata = chXdata & 0b00011110;

      chAddress = chYdata | chXdata;

      if (setOrClear == 1)
      {
        chAddress = chAddress | 0b00000001; // this last bit determines whether we set or unset the path
      }

      chAddress = chAddress << 24;

      // delayMicroseconds(50);

      delayMicroseconds(20);

      pio_sm_put(pio, sm, chAddress);

      delayMicroseconds(40);
      //}
    }
  }
}

void createXYarray(void)
{
}
