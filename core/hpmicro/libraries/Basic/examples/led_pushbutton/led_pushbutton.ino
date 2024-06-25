#include "board.h"
#include "Arduino.h"

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_PUSH_BUTTON, INPUT);
}

void loop() {
    printf("Button is %s\n", digitalRead(PIN_PUSH_BUTTON) == LOW ? "PRESSED" : "RELEASED");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}