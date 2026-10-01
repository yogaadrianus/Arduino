#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;


// =====================================================
// HEART RATE
// =====================================================

const byte RATE_SIZE = 4;

byte rates[RATE_SIZE];
byte rateSpot = 0;
byte rateCount = 0;

long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;


// =====================================================
// SpO2
// =====================================================

#define SAMPLE_COUNT 50

long redSum = 0;
long irSum = 0;

long redMin = 999999;
long redMax = 0;

long irMin = 999999;
long irMax = 0;

int sampleCount = 0;

int spo2 = 0;


// =====================================================
// TIMING
// =====================================================

unsigned long lastDisplay = 0;
unsigned long lastSerial = 0;


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(9600);

  // ---------------- OLED ----------------

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);

  display.setCursor(5, 10);
  display.print("HR: -- BPM");

  display.setCursor(5, 40);
  display.print("Oxy: -- %");

  display.display();

  delay(1000);


  // ---------------- MAX30102 ----------------

  particleSensor.begin(Wire, I2C_SPEED_FAST);

  particleSensor.setup();

  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  particleSensor.clearFIFO();
}


// =====================================================
// RESET MEASUREMENT
// =====================================================

void resetMeasurement()
{
  // HR

  beatAvg = 0;
  beatsPerMinute = 0;

  rateSpot = 0;
  rateCount = 0;

  lastBeat = 0;


  // SpO2

  spo2 = 0;

  redSum = 0;
  irSum = 0;

  redMin = 999999;
  redMax = 0;

  irMin = 999999;
  irMax = 0;

  sampleCount = 0;
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  /*
     Ambil sample terbaru dari MAX30102
  */

  particleSensor.check();


  /*
     Proses semua sample yang tersedia
  */

  while (particleSensor.available())
  {
    long irValue = particleSensor.getIR();
    long redValue = particleSensor.getRed();


    // =================================================
    // NO FINGER
    // =================================================

    if (irValue < 7000)
    {
      resetMeasurement();
    }


    // =================================================
    // FINGER DETECTED
    // =================================================

    else
    {

      // =================================================
      // HEART RATE
      // =================================================

      if (checkForBeat(irValue))
      {
        unsigned long now = millis();


        /*
           Jika sudah pernah mendeteksi beat sebelumnya,
           hitung jarak antar beat.
        */

        if (lastBeat > 0)
        {
          long delta = now - lastBeat;


          /*
             Hitung BPM
          */

          beatsPerMinute =
              60.0 / (delta / 1000.0);


          /*
             Buang nilai yang tidak masuk akal
          */

          if (beatsPerMinute > 30 &&
              beatsPerMinute < 220)
          {

            // -----------------------------------------
            // SIMPAN BPM
            // -----------------------------------------

            rates[rateSpot] =
                (byte)beatsPerMinute;

            rateSpot++;

            if (rateSpot >= RATE_SIZE)
            {
              rateSpot = 0;
            }


            if (rateCount < RATE_SIZE)
            {
              rateCount++;
            }


            // -----------------------------------------
            // CEPAT TAMPILKAN BPM
            // -----------------------------------------

            /*
               Sebelum 4 beat terkumpul,
               gunakan BPM terbaru.

               Jadi HR tidak perlu menunggu
               4 detak untuk mulai tampil.
            */

            if (rateCount < RATE_SIZE)
            {
              beatAvg =
                  (int)beatsPerMinute;
            }


            // -----------------------------------------
            // SETELAH 4 BEAT → AVERAGE
            // -----------------------------------------

            else
            {
              int total = 0;

              for (byte i = 0;
                   i < RATE_SIZE;
                   i++)
              {
                total += rates[i];
              }

              beatAvg =
                  total / RATE_SIZE;
            }
          }
        }


        /*
           Simpan waktu beat terakhir
        */

        lastBeat = now;
      }


      // =================================================
      // SpO2
      // =================================================

      if (redValue < redMin)
      {
        redMin = redValue;
      }

      if (redValue > redMax)
      {
        redMax = redValue;
      }

      if (irValue < irMin)
      {
        irMin = irValue;
      }

      if (irValue > irMax)
      {
        irMax = irValue;
      }


      redSum += redValue;
      irSum += irValue;

      sampleCount++;


      // =================================================
      // CALCULATE SpO2
      // =================================================

      if (sampleCount >= SAMPLE_COUNT)
      {
        long redDC =
            redSum / SAMPLE_COUNT;

        long irDC =
            irSum / SAMPLE_COUNT;

        long redAC =
            redMax - redMin;

        long irAC =
            irMax - irMin;


        if (redDC > 0 &&
            irDC > 0 &&
            redAC > 0 &&
            irAC > 0)
        {
          float R;


          R =
              ((float)redAC / redDC) /
              ((float)irAC / irDC);


          /*
             Estimasi SpO2
          */

          spo2 =
              110 - (25 * R);


          /*
             Batasi 0 - 100%
          */

          if (spo2 > 100)
          {
            spo2 = 100;
          }

          if (spo2 < 0)
          {
            spo2 = 0;
          }
        }
        else
        {
          spo2 = 0;
        }


        // -----------------------------------------
        // RESET SAMPLE SpO2
        // -----------------------------------------

        redSum = 0;
        irSum = 0;

        redMin = 999999;
        redMax = 0;

        irMin = 999999;
        irMax = 0;

        sampleCount = 0;
      }
    }


    /*
       Pindah ke sample berikutnya
    */

    particleSensor.nextSample();
  }


  // =====================================================
  // OLED
  // =====================================================

  /*
     Update OLED setiap 200 ms.
     HR tidak perlu menunggu update SpO2.
  */

  if (millis() - lastDisplay >= 200)
  {
    lastDisplay = millis();


    display.clearDisplay();

    display.setTextColor(WHITE);
    display.setTextSize(2);


    // ---------------- HR ----------------

    display.setCursor(5, 10);

    display.print("HR: ");

    if (beatAvg > 0)
    {
      display.print(beatAvg);
    }
    else
    {
      display.print("--");
    }

    display.print(" BPM");


    // ---------------- SpO2 ----------------

    display.setCursor(5, 40);

    display.print("Oxy: ");

    if (spo2 > 0)
    {
      display.print(spo2);
    }
    else
    {
      display.print("--");
    }

    display.print(" %");


    display.display();
  }


  // =====================================================
  // SERIAL MONITOR
  // =====================================================

  /*
     Hanya print setiap 1 detik supaya Serial Monitor
     tidak memperlambat pembacaan sensor.
  */

  if (millis() - lastSerial >= 1000)
  {
    lastSerial = millis();

    Serial.print("HR: ");
    Serial.print(beatAvg);

    Serial.print(" BPM | Oxy: ");
    Serial.print(spo2);

    Serial.print(" % | IR: ");
    Serial.println(particleSensor.getIR());
  }
}

