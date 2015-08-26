#include "application.h"

#include "neopixel.h"
#define PIXEL_COUNT 130
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, D0, WS2812B);


#include "elapsedMillis.h"
elapsedMillis timerDHT22;
elapsedMillis timerEffect;
uint32_t intervalEffect;


#include "PietteTech_DHT.h"
void dht_wrapper();
PietteTech_DHT DHT(D2, DHT22, dht_wrapper);
bool dhtStarted = false;
String dhtError = "";
uint32_t dhtTimestamp = 0;
double dhtFahrenheit = 0;
double dhtHumidity = 0;
double dhtDewPoint = 0;


void doWord(const uint8_t *w);
void undoWord(const uint8_t *w);
void randomColor();
void blackOut();
void displayDigit(uint8_t d, byte n);
void displayDigit(uint8_t d, byte n, uint8_t c[3]);
void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
void doDHT22();
void doTime();
void displayDefaultText();
void displayHour();
void displayMinute();

uint8_t color[3] = {0, 0, 64};

static const uint8_t wordIt[8] = {117, 118};
static const uint8_t wordIs[8] = {120, 121};
static const uint8_t wordTen[8] = {123, 124, 125};
static const uint8_t wordHalf[8] = {126, 127, 128, 129};
static const uint8_t wordQuarter[8] = {116, 115, 114, 113, 112, 111, 110};
static const uint8_t wordTwenty[8] = {109, 107, 108, 106, 105, 104};
static const uint8_t wordFive[8] = {91, 92, 93, 94};
static const uint8_t wordMinutes[8] = {96, 97, 98, 99, 100, 101, 102};
static const uint8_t wordHappy[8] = {90, 89, 88, 87, 86};
static const uint8_t wordPast[8] = {84, 83, 82, 81};
static const uint8_t wordTo[8] = {79, 78};
static const uint8_t wordSEVEN[8] = {65, 66, 67, 68, 69};
static const uint8_t wordBirthday[8] = {70, 71, 72, 73, 74, 75, 76, 77};
static const uint8_t wordELEVEN[8] = {64, 63, 62, 61, 60, 59};
static const uint8_t wordNINE[8] = {59, 58, 57, 56};
static const uint8_t wordSIX[8] = {54, 53, 52};
static const uint8_t wordTWO[8] = {41, 40, 39};
static const uint8_t wordONE[8] = {43, 42, 41};
static const uint8_t wordEIGHT[8] = {47, 46, 45, 44, 43};
static const uint8_t wordTHREE[8] = {51, 50, 49, 48, 47};
static const uint8_t wordCora[8] = {38, 37, 36, 35};
static const uint8_t wordFIVE[8] = {34, 33, 32, 31};
static const uint8_t wordFOUR[8] = {29, 28, 27, 26};
static const uint8_t wordLola[8] = {16, 15, 14, 13};
static const uint8_t wordTEN[8] = {19, 18, 17};
static const uint8_t wordMom[8] = {22, 21, 20};
static const uint8_t wordDad[8] = {25, 24, 23};
static const uint8_t wordTWELVE[8] = {12, 11, 10, 9, 8, 7};
static const uint8_t wordOClock[8] = {6, 5, 4, 3, 2, 1, 0};


// EEPROM
// Address 0 = 117 if values have been saved
// Address 1 = 0/1 for -/+ of time zone offset
// Address 2 = Time zone offset (positive integer)
// Address 3 = 12/24 for hour format
// Address 4 = Effect mode
// Address 5 = Red
// Address 6 = Green
// Address 7 = Blue
// Address 8 = Rainbow delay

int8_t timeZone = 0;
bool time12Hour = false;
bool resetFlag = false;
elapsedMillis timerReset = 0;

// Effect modes
// 0 = no effect
// 1 = rainbow
// 2 = display local (indoor) environmentals
// 3 = display outdoor environmentals
// 4 = cycle modes (time, local envs, outdoor envs)
uint8_t EFFECT_MODE = 0;
uint8_t LAST_EFFECT_MODE = EFFECT_MODE;
uint8_t currEffect = EFFECT_MODE;
uint16_t RAINBOW_DELAY = 50;
uint8_t LAST_MINUTE = 0;


void setup() {
strip.begin();
    strip.show();

    for(uint8_t i=0; i<PIXEL_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));

        if(i>0)
            strip.setPixelColor(i-1, strip.Color(0, 0, 0));

        strip.show();
        delay(10);
    }

    Spark.variable("Fahrenheit", &dhtFahrenheit, DOUBLE);
    Spark.variable("Humidity", &dhtHumidity, DOUBLE);
    Spark.variable("DewPoint", &dhtDewPoint, DOUBLE);
    Spark.variable("dhtError", &dhtError, STRING);
    Spark.variable("dhtTS", &dhtTimestamp, INT);

    Spark.function("function", fnRouter);

    // See if this EEPROM has saved data
    if(EEPROM.read(0)==117) {
        // Set the time zone
        if(EEPROM.read(1)==0)
            timeZone = EEPROM.read(2)*-1;
        else
            timeZone = EEPROM.read(2);

        // Set the hour format
        if(EEPROM.read(3)==12)
            time12Hour = true;
        else
            time12Hour = false;

        EFFECT_MODE = EEPROM.read(4);
        LAST_EFFECT_MODE = EFFECT_MODE;

        color[0] = EEPROM.read(5);
        color[1] = EEPROM.read(6);
        color[2] = EEPROM.read(7);

        RAINBOW_DELAY = EEPROM.read(8);

    // If data has not been saved, "initialize" the EEPROM
    } else {
        // Initialize
        EEPROM.write(0, 117);
        // Time zone +/-
        EEPROM.write(1, 0);
        // Time zone
        EEPROM.write(2, 0);
        // Hour format
        EEPROM.write(3, 24);
        // Effect mode
        EEPROM.write(4, 0);
        // Red
        EEPROM.write(5, 0);
        // Green
        EEPROM.write(6, 255);
        // Blue
        EEPROM.write(7, 128);
        // Rainbow delay
        EEPROM.write(8, RAINBOW_DELAY);
    }

    Time.zone(timeZone);

    blackOut();
    strip.show();
}


void loop() {
    doEffectMode();
    doDHT22();

    if(timerReset>=500) {
        if(resetFlag) {
            System.reset();
            resetFlag = false;
        }

        timerReset = 0;
    }
}


int fnRouter(String command) {
    command.trim();
    command.toUpperCase();

    // Get time zone offset
    if(command.equals("GETTIMEZONE")) {
        return timeZone;


    // Set time zone offset
    } else if(command.substring(0, 12)=="SETTIMEZONE,") {
        timeZone = command.substring(12).toInt();
        Time.zone(timeZone);

        if(timeZone>-1) {
            EEPROM.write(1, 1);
            EEPROM.write(2, timeZone);
        } else {
            EEPROM.write(1, 0);
            EEPROM.write(2, timeZone * -1);
        }

        intervalEffect = 0;
        return timeZone;


    // Lazy way to reboot
    } else if(command.equals("REBOOT")) {
        resetFlag = true;
        return 1;


    // Set red
    } else if(command.substring(0, 7)=="SETRED,") {
        color[0] = command.substring(7).toInt();
        EEPROM.write(5, color[0]);
        intervalEffect = 0;

        return color[0];


    // Set green
    } else if(command.substring(0, 9)=="SETGREEN,") {
        color[1] = command.substring(9).toInt();
        EEPROM.write(6, color[1]);
        intervalEffect = 0;

        return color[1];


    // Set blue
    } else if(command.substring(0, 8)=="SETBLUE,") {
        color[2] = command.substring(8).toInt();
        EEPROM.write(7, color[2]);
        intervalEffect = 0;

        return color[2];


    // Set RGB
    } else if(command.substring(0, 7)=="SETRGB,") {
        color[0] = command.substring(7, 10).toInt();
        color[1] = command.substring(11, 14).toInt();
        color[2] = command.substring(15, 18).toInt();

        EEPROM.write(5, color[0]);
        EEPROM.write(6, color[1]);
        EEPROM.write(7, color[2]);

        intervalEffect = 0;

        return 1;


    // Random color
    } else if(command.equals("RANDOMCOLOR")) {
        randomColor();
        intervalEffect = 0;

        return 1;


    // Set effect mode
    } else if(command.substring(0, 10)=="SETEFFECT,") {
        EFFECT_MODE = command.substring(10).toInt();
        EEPROM.write(4, EFFECT_MODE);
        intervalEffect = 0;

        return EFFECT_MODE;


    // Get effect mode
    } else if(command.equals("GETEFFECTMODE")) {
        return EFFECT_MODE;


    // Get pixel color
    } else if(command.substring(0, 14)=="GETPIXELCOLOR,") {
        return strip.getPixelColor(command.substring(14).toInt());


    // Set rainbow effect delay
    } else if(command.substring(0, 16)=="SETRAINBOWDELAY,") {
        RAINBOW_DELAY = command.substring(16).toInt();
        intervalEffect = RAINBOW_DELAY;
        EEPROM.write(8, RAINBOW_DELAY);
        return RAINBOW_DELAY;

    // Get rainbow effect delay
    } else if(command.equals("GETRAINBOWDELAY")) {
        return RAINBOW_DELAY;


    // Display a word
    } else if(command.substring(0,7)=="DOWORD,") {
        String w = command.substring(7);
        if(w.equals("IT"))
            doWord(wordIt);
        else if(w.equals("IS"))
            doWord(wordIs);
        else if(w.equals("TEN"))
            doWord(wordTen);
        else if(w.equals("HALF"))
            doWord(wordHalf);
        else if(w.equals("QUARTER"))
            doWord(wordQuarter);
        else if(w.equals("TWENTY"))
            doWord(wordTwenty);
        else if(w.equals("FIVE"))
            doWord(wordFive);
        else if(w.equals("MINUTES"))
            doWord(wordMinutes);
        else if(w.equals("HAPPY"))
            doWord(wordHappy);
        else if(w.equals("PAST"))
            doWord(wordPast);
        else if(w.equals("TO"))
            doWord(wordTo);
        else if(w.equals("SEVEN"))
            doWord(wordSEVEN);
        else if(w.equals("BIRTHDAY"))
            doWord(wordBirthday);
        else if(w.equals("ELEVEN"))
            doWord(wordELEVEN);
        else if(w.equals("NINE"))
            doWord(wordNINE);
        else if(w.equals("SIX"))
            doWord(wordSIX);
        else if(w.equals("TWO"))
            doWord(wordTWO);
        else if(w.equals("ONE"))
            doWord(wordONE);
        else if(w.equals("EIGHT"))
            doWord(wordEIGHT);
        else if(w.equals("THREE"))
            doWord(wordTHREE);
        else if(w.equals("CORA"))
            doWord(wordCora);
        else if(w.equals("FIVE2"))
            doWord(wordFIVE);
        else if(w.equals("FOUR"))
            doWord(wordFOUR);
        else if(w.equals("LOLA"))
            doWord(wordLola);
        else if(w.equals("TEN2"))
            doWord(wordTEN);
        else if(w.equals("MOM"))
            doWord(wordMom);
        else if(w.equals("DAD"))
            doWord(wordDad);
        else if(w.equals("TWELVE"))
            doWord(wordTWELVE);
        else if(w.equals("OCLOCK") || w.equals("O'CLOCK"))
            doWord(wordOClock);

        return command.length()-7;


    // Un-display a word
    } else if(command.substring(0,9)=="UNDOWORD,") {
        String w = command.substring(9);
        if(w.equals("IT"))
            undoWord(wordIt);
        else if(w.equals("IS"))
            undoWord(wordIs);
        else if(w.equals("TEN"))
            undoWord(wordTen);
        else if(w.equals("HALF"))
            undoWord(wordHalf);
        else if(w.equals("QUARTER"))
            undoWord(wordQuarter);
        else if(w.equals("TWENTY"))
            undoWord(wordTwenty);
        else if(w.equals("FIVE"))
            undoWord(wordFive);
        else if(w.equals("MINUTES"))
            undoWord(wordMinutes);
        else if(w.equals("HAPPY"))
            undoWord(wordHappy);
        else if(w.equals("PAST"))
            undoWord(wordPast);
        else if(w.equals("TO"))
            undoWord(wordTo);
        else if(w.equals("SEVEN"))
            undoWord(wordSEVEN);
        else if(w.equals("BIRTHDAY"))
            undoWord(wordBirthday);
        else if(w.equals("ELEVEN"))
            undoWord(wordELEVEN);
        else if(w.equals("NINE"))
            undoWord(wordNINE);
        else if(w.equals("SIX"))
            undoWord(wordSIX);
        else if(w.equals("TWO"))
            undoWord(wordTWO);
        else if(w.equals("ONE"))
            undoWord(wordONE);
        else if(w.equals("EIGHT"))
            undoWord(wordEIGHT);
        else if(w.equals("THREE"))
            undoWord(wordTHREE);
        else if(w.equals("CORA"))
            undoWord(wordCora);
        else if(w.equals("FIVE2"))
            undoWord(wordFIVE);
        else if(w.equals("FOUR"))
            undoWord(wordFOUR);
        else if(w.equals("LOLA"))
            undoWord(wordLola);
        else if(w.equals("TEN2"))
            undoWord(wordTEN);
        else if(w.equals("MOM"))
            undoWord(wordMom);
        else if(w.equals("DAD"))
            undoWord(wordDad);
        else if(w.equals("TWELVE"))
            undoWord(wordTWELVE);
        else if(w.equals("OCLOCK") || w.equals("O'CLOCK"))
            undoWord(wordOClock);

        return command.length()-9;
    }


    return -1;
}


void randomColor() {
    color[0] = random(32, 255);
    color[1] = random(32, 255);
    color[1] = random(32, 255);
}


void blackOut() {
    // Black it out
    for(uint8_t x=0; x<PIXEL_COUNT; x++)
        strip.setPixelColor(x, strip.Color(0, 0, 0));
}


void doWord(const uint8_t *w) {
    for(uint8_t i=0; i<sizeof(w)*2; i++)
        strip.setPixelColor(w[i], strip.Color(color[0], color[1], color[2]));
}


void undoWord(const uint8_t *w) {
    for(uint8_t i=0; i<sizeof(w)*2; i++)
        strip.setPixelColor(w[i], strip.Color(0, 0, 0));
}


void rainbow(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256; j++) {
        for(i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i+j) & 255));
        }

        strip.show();
        delay(wait);
    }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}


void doDHT22() {
    if(timerDHT22>2000) {
        if(!dhtStarted) {
            DHT.acquire();
            dhtStarted = true;
        }

        if(!DHT.acquiring()) {
            int dhtResult = DHT.getStatus();

            switch (dhtResult) {
                case DHTLIB_OK:
                    dhtError = "";
                    break;
                case DHTLIB_ERROR_CHECKSUM:
                    dhtError = "Checksum";
                    break;
                case DHTLIB_ERROR_ISR_TIMEOUT:
                    dhtError = "ISR timeout";
                    break;
                case DHTLIB_ERROR_RESPONSE_TIMEOUT:
                    dhtError = "Response timeout";
                    break;
                case DHTLIB_ERROR_DATA_TIMEOUT:
                    dhtError = "Data timeout";
                    break;
                case DHTLIB_ERROR_ACQUIRING:
                    dhtError = "Acquiring";
                    break;
                case DHTLIB_ERROR_DELTA:
                    dhtError = "Delta time to small";
                    break;
                case DHTLIB_ERROR_NOTSTARTED:
                    dhtError = "Not started";
                    break;
                default:
                    dhtError = "Unknown";
                    break;
            }

            if(dhtResult==DHTLIB_OK) {
                dhtTimestamp = Time.now();

                dhtHumidity = DHT.getHumidity();
                dhtFahrenheit = DHT.getFahrenheit();
                dhtDewPoint = DHT.getDewPoint();

                //String pub = "{\"h\":" + String((float)dhtHumidity) + ",\"f\":" + String((float)dhtFahrenheit) + ",\"d\":" + String((float)dhtDewPoint) + "}";
                //Spark.publish("environmentals", pub, 2);

                String pub = "h:"+String((float)dhtHumidity)+"|g,f:"+String((float)dhtFahrenheit)+"|g";
                Spark.publish("statsd", pub, 60);
            }

            dhtStarted = false;
        }

        timerDHT22 = 0;
    }
}


void doTime() {
    blackOut();

    displayDefaultText();
    displayHour();
    displayMinute();

    strip.show();
}


void displayDefaultText() {
    // it
    doWord(wordIt);

    // is
    doWord(wordIs);

    // o'clock
    doWord(wordOClock);
}


void displayHour() {
    int h = Time.hourFormat12();
    int m = Time.minute();

    // hours
    //if(h==0)
    //    h = 12;


    // Past / to
    if(m>=4 && m<=33)
        doWord(wordPast);
    else if(m>=34) {
        h++;
        
        if(h==13)
            h = 1;

        //if(h==0 || h==12)
        //    h = 1;

        doWord(wordTo);
    }


    // hour
    switch(h) {
        case 1:
            doWord(wordONE);
            break;
        case 2:
            doWord(wordTWO);
            break;
        case 3:
            doWord(wordTHREE);
            break;
        case 4:
            doWord(wordFOUR);
            break;
        case 5:
            doWord(wordFIVE);
            break;
        case 6:
            doWord(wordSIX);
            break;
        case 7:
            doWord(wordSEVEN);
            break;
        case 8:
            doWord(wordEIGHT);
            break;
        case 9:
            doWord(wordNINE);
            break;
        case 10:
            doWord(wordTEN);
            break;
        case 11:
            doWord(wordELEVEN);
            break;
        case 12:
            doWord(wordTWELVE);
            break;
        default: // It seems that I can't get this correct the right way
            doWord(wordONE);
    }
}


void displayMinute() {
    int h = Time.hourFormat12();
    int m = Time.minute();

    // hours
    if(h==0)
        h = 12;


    // Past / to
    if(m>=4 && m<=33)
        doWord(wordPast);
    else if(m>=34) {
        h++;

        doWord(wordTo);
    }


    // Five minutes
    if(m>=4 && m<=8) {
        doWord(wordFive);
        doWord(wordMinutes);

    // Ten minutes
    } else if(m>=9 && m<=12) {
        doWord(wordTen);
        doWord(wordMinutes);

    // Quarter
    } else if(m>=13 && m<=18) {
        doWord(wordQuarter);

    // Twenty
    } else if(m>=19 && m<=22) {
        doWord(wordTwenty);
        doWord(wordMinutes);

    // Twenty five
    } else if (m>=23 && m<=27) {
        doWord(wordTwenty);
        doWord(wordFive);
        doWord(wordMinutes);

    // Half past
    } else if(m>=28 && m<=32) {
        doWord(wordHalf);

    // Twenty five
    } else if(m>=33 && m<=37) {
        doWord(wordTwenty);
        doWord(wordFive);
        doWord(wordMinutes);

    // Twenty
    } else if(m>=38 && m<=42) {
        doWord(wordTwenty);
        doWord(wordMinutes);

    // Quarter
    } else if(m>=41 && m<=47) {
        doWord(wordQuarter);

    // Ten minutes
    } else if(m>=48 && m<=52) {
        doWord(wordTen);
        doWord(wordMinutes);

    // Five minutes
    } else if(m>=53) {
        doWord(wordFive);
        doWord(wordMinutes);
    }
}


void doEffectMode() {
    if(EFFECT_MODE!=LAST_EFFECT_MODE) {
        blackOut();
        LAST_EFFECT_MODE = EFFECT_MODE;
    }

    if(timerEffect>=intervalEffect) {
        timerEffect = 0;

        switch(EFFECT_MODE) {
            case 1: // Rainbow
                doEffectRainbow();
                intervalEffect = RAINBOW_DELAY;
                break;

            //case 2: // Temp/humidity
            //    doEffectEnvironmentals(true);
            //    intervalEffect = 2000;
            //    break;

            //case 3: // Internet temp/humidity
            //    doEffectEnvironmentals(false);
            //    kchaInterval = 0;
            //    intervalEffect = 1000;
            //    break;

            //case 4: // Cycle
                //doTime();
                //intervalEffect = 5000;

            case 0: // Time
            default:
                doTime();
                intervalEffect = 1000;
        }
    }
}


void doEffectRainbow() {
    uint16_t i, j;

    if(Time.minute()!=LAST_MINUTE) {
        blackOut();
        LAST_MINUTE = Time.minute();
    }

    displayDefaultText();
    displayHour();
    displayMinute();

    for(j=0; j<256; j++) {
        if(EFFECT_MODE!=1) break;

        for(i=0; i<strip.numPixels(); i++) {
            if(strip.getPixelColor(i)>0)
                strip.setPixelColor(i, Wheel((i+j) & 255));
        }

        strip.show();
        delay(RAINBOW_DELAY);
        Spark.process();
    }
}


void dht_wrapper() {
    DHT.isrCallback();
}