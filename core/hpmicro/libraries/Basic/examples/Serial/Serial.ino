#include "board.h"
#include "Arduino.h"

int num = 100;

void setup() {
    Serial.begin(115200);
}

void loop() {
    num += 100;
    Serial.write('A');
    Serial.write("BCDE\n");
    Serial.println(num, 2);
    Serial.println(num, 8);
    Serial.println(num, 10);
    Serial.println(num, 16);
    if (Serial.available()) {
        Serial.write("Serial Recieve Data:\n");
        while (Serial.available()) {
            Serial.write(Serial.read());
        }
        Serial.print('\n');
    }
    delay(500);
}