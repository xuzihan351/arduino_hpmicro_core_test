# ButtonMouseControl

There are a total of 5 GPIO inputs, simulating mouse movement or clicked.

|GPIO      |Function      |
|----      |----      |
| PIN_P1_7 | upButton |
| PIN_P1_8 | downButton |
| PIN_P1_11 | leftButton |
| PIN_P1_12 | rightButton |
| PIN_P1_10 | mouseButton |

When GPIO is connected to a high level, it indicates that the mouse is moving or clicked. For example, if upButton is connected to a high level, the mouse will move up.

