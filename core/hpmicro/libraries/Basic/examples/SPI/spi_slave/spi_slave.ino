#include <SPI.h>
char buff[50];
volatile byte indx;
volatile boolean process;
void spi1_callback(void *para);

void setup() {
  Serial.begin(115200);
  SPI.begin(SPI_SLAVE_ROLE);
  indx = 0;
  process = false;
  SPI.attachInterruptParam(spi1_callback, 0);
  SPI.usingInterrupt(IRQn_SPI1);
}

void spi1_callback(void *para) {
  uint8_t *status = (uint8_t *)para;
  if ((*status) == SPI_VALID_RX_FLAG) {
    if (indx < sizeof(buff)) {
      buff[indx++] = SPI.read_byte();
      if (buff[indx - 1] == '\r') {
        process = true;
      }
    }
  }
  if ((*status) == SPI_EMPTY_TX_FLAG) {
    SPI.write_byte(0x00); /* dummy write */
  }
  if ((*status) == SPI_END_OF_TRANSFER_FLAG) {
    process = true;
  }
}


void loop() {
  if (process) {
    process = false;
    buff[indx - 1] = 0;
    Serial.println(buff);
    indx = 0;
  }
}
