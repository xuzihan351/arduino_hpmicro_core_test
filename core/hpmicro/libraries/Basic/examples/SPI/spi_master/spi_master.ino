#include <SPI.h>

#define SAMPLE_PIN PIN_SPI_SS

void setup() {
  Serial.begin(115200);
  pinMode(SAMPLE_PIN, OUTPUT);
  digitalWrite(SAMPLE_PIN, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);
}

void loop() {
  char c;
  digitalWrite(SAMPLE_PIN, LOW);

  for (const char* p = "Hello, world!\r"; c = *p; p++) {
    SPI.transfer(c);
    Serial.print(c);
  }
  digitalWrite(SAMPLE_PIN, HIGH);
  delay(2000);
}