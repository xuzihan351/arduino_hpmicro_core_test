#include <Wire.h>
void setup()
{
    /* Wire is initialized and joins the i2c bus as slave device address #8 */
    Wire.begin(8);
    /* Register request response event */
    Wire.onRequest(requestEvent);
}
 
void loop()
{
    delay(100);
}
 
void requestEvent()
{
    Wire.write("hello ");
}