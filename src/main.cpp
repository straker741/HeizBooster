#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// https://forum.arduino.cc/index.php?topic=415167.0
extern "C" {
    #include "ATmegaTimers.h"
}

enum State {HOT, WARM, COLD, ERROR, STATE_COUNT};
State state = State::COLD;

#define ONE_WIRE_BUS  11
#define Q1_PIN        12
#define ERROR_PIN     13

//#define DEBUGGING              1         // comment out when not used
#define DELAY                  1000      // Miliseconds
#define MAX_TEMP_THRESHOLD     34.75     // MUST BE GIVEN AS A MULTIPLY BY 0.25 !!!
#define DELTA_TEMP             3.5
#define MIN_FAN_PERCENTAGE     30.0
#define MAX_CONSECUTIVE_ERRORS 5

float dc = 0.0;
float temps[5];
float maxTemp = 0;
float minTemp = 0;
uint8_t numberOfSensors = 0;
uint8_t i = 0;
uint8_t errors = 0;

const float MIN_TEMP_THRESHOLD = MAX_TEMP_THRESHOLD - DELTA_TEMP;
const float k = (float)(100.0 - MIN_FAN_PERCENTAGE) / (float)DELTA_TEMP;
const float q = 100.0 - k * MAX_TEMP_THRESHOLD;
const uint16_t SHORT_DELAY = DELAY >> 3;

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
// Pass our oneWire reference to Dallas Temperature. 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


void setup() {
    pinMode(Q1_PIN, OUTPUT);
    digitalWrite(Q1_PIN, LOW);
    state = State::COLD;

    pinMode(ERROR_PIN, OUTPUT);
    digitalWrite(ERROR_PIN, LOW);

    #ifdef DEBUGGING
    Serial.begin(115200);
    Serial.println("HeizBooster!");
    #endif

    PWM_init(25000, 16000000);    // Resolution: 1/80 * 100(%) == 1.25%
    PWM_setDutyCycle(100);        // at 0% there will always be narrow spike
}


void loop() {
    // Initialization of the bus
    sensors.begin();
    // Get number of sensors present on the bus
    numberOfSensors = sensors.getDS18Count();

    if (numberOfSensors == 0) {
        state = State::ERROR;
        #ifdef DEBUGGING
        Serial.println("Have not found any DS18xxx device!");
        #endif
    }
    else if (numberOfSensors > 5) {
        state = State::ERROR;
        #ifdef DEBUGGING
        Serial.println("Found too many DS18xxx devices!");
        #endif
    }
    else {
        #ifdef DEBUGGING
        Serial.print("Number of found DS18xxx devices: ");
        Serial.println(numberOfSensors);
        #endif

        // call sensors.requestTemperatures() to issue a global temperature
        // request to all devices on the bus
        sensors.requestTemperatures(); // Send the command to get temperature readings

        // Saving temperatures
        for (i = 0; i < numberOfSensors; i++) {
            temps[i] = sensors.getTempCByIndex(i);
        }

        #ifdef DEBUGGING
        Serial.print("temps: [");
        for (i = 0; i < numberOfSensors; i++) {
            Serial.print(temps[i]);
            if (i+1 != numberOfSensors) {
                Serial.print(", ");
            }
        }
        Serial.println("]");
        #endif

        // Finding maximum and minimum of saved temperatures
        minTemp = maxTemp = temps[0];
        for (i = 1; i < numberOfSensors; i++) {
            if (maxTemp < temps[i]) {
                maxTemp = temps[i];
            }
            if (minTemp > temps[i]) {
                minTemp = temps[i];
            }
        }

        // Determine the state based on the temperature
        // Check if there is incorrect value like DEVICE_DISCONNECTED_C
        if (minTemp == DEVICE_DISCONNECTED_C) {
            state = State::ERROR;
        }
        else {
            if (MAX_TEMP_THRESHOLD <= maxTemp) {
                // temp: <34.75, +inf)
                state = State::HOT;
            }
            else if ((MIN_TEMP_THRESHOLD <= maxTemp) && (maxTemp < MAX_TEMP_THRESHOLD)) {
                // temp: <31.25, 34.75)
                state = State::WARM;
            }
            else {
                // temp: (-inf, 31.25)
                state = State::COLD;
            }
        }

    }

    // Take action based on the state
    switch(state) {
        case State::HOT:
            errors = 0;
            digitalWrite(Q1_PIN, HIGH);
            PWM_setDutyCycle(100);
            break;
        case State::WARM:
            errors = 0;
            digitalWrite(Q1_PIN, HIGH);

            dc = k * maxTemp + q;
            dc = PWM_setDutyCycle(dc);

            #ifdef DEBUGGING
            Serial.print("Running at ");
            Serial.print(dc);
            Serial.println("%");
            #endif
            break;
        case State::COLD:
            errors = 0;
            digitalWrite(Q1_PIN, LOW);
            PWM_setDutyCycle(100); // No narrow spike at the OCR2B == TOP
            // ??? Maybe turn off clock for timer2?
            break;
        case State::ERROR:
        default:
            errors++;
            if (errors > MAX_CONSECUTIVE_ERRORS) {
                digitalWrite(Q1_PIN, LOW); // Turn off fans
                errors = 0;
            }
            for (i = 0; i < 4; i++) {
                digitalWrite(ERROR_PIN, HIGH);
                delay(SHORT_DELAY);
                digitalWrite(ERROR_PIN, LOW);
                delay(SHORT_DELAY);
            }
            break;
    }
    delay(DELAY);
}
