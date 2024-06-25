# ButtonMouseControl

总共有5个gpio输入，模拟鼠标移动和点击。

|GPIO      |功能      |
|----      |----      |
| PIN_P1_7 | upButton |
| PIN_P1_8 | downButton |
| PIN_P1_11 | leftButton |
| PIN_P1_12 | rightButton |
| PIN_P1_10 | mouseButton |

当GPIO接高电平时，表示鼠标移动或按下。例如upButton接高电平，则鼠标向上移动。

