#include "USBMouse.h"

// set pin numbers for the five buttons:
const int upButton = PIN_P1_7;
const int downButton = PIN_P1_8;
const int leftButton = PIN_P1_11;
const int rightButton = PIN_P1_12;
const int mouseButton = PIN_P1_10;

int range = 5;           // output range of X or Y movement; affects movement speed
int responseDelay = 10;  // response delay of the mouse, in ms

void setup() {
  // initialize the buttons' inputs:
  pinMode(upButton, INPUT_PULLDOWN);
  pinMode(downButton, INPUT_PULLDOWN);
  pinMode(leftButton, INPUT_PULLDOWN);
  pinMode(rightButton, INPUT_PULLDOWN);
  pinMode(mouseButton, INPUT_PULLDOWN);
  // initialize mouse control:
  USBMouse.begin();
}

void loop() {
  // read the buttons:
  int upState = digitalRead(upButton);
  int downState = digitalRead(downButton);
  int rightState = digitalRead(rightButton);
  int leftState = digitalRead(leftButton);
  int clickState = digitalRead(mouseButton);

  // calculate the movement distance based on the button states:
  int xDistance = (rightState - leftState) * range;
  int yDistance = (downState - upState) * range;

  // printf("upState = %d, downState = %d, rightState = %d, leftState = %d, clickState = %d, xDistance = %d, yDistance = %d\n",
  //         upState, downState, rightState, leftState, clickState, xDistance, yDistance);

  // if X or Y is non-zero, move:
  if ((xDistance != 0) || (yDistance != 0)) {
    USBMouse.move(xDistance, yDistance, 0);
  }

  // if the mouse button is pressed:
  if (clickState == HIGH) {
    // if the mouse is not pressed, press it:
    if (!USBMouse.isPressed(MOUSE_LEFT)) {
      USBMouse.press(MOUSE_LEFT);
    }
  }
  // else the mouse button is not pressed:
  else {
    // if the mouse is pressed, release it:
    if (USBMouse.isPressed(MOUSE_LEFT)) {
      USBMouse.release(MOUSE_LEFT);
    }
  }

  // a delay so the mouse doesn't move too fast:
  delay(responseDelay);
}
