# Linux Driver for the LCD 1602

The following is an attempt at writing a Linux Device Driver for the LCD1602. The intent is to use this with an off the shelf Raspberry Pi Model 3B.

The GPIO selection will be using the pin selection listed [here](http://wiki.sunfounder.cc/index.php?title=LCD1602_Module) and are the following:

Note: The pin number does NOT match the GPIO label assigned to that pin. According to several online diagrams, the selected pins correspond to GPIO_GEN1 through GPIO_GEN6

| LCD 1602 Pin | Raspberry Pi Pin |
|--------------|------------------|
| K            | GND              |
| A            | 3.3V             |
| D7           | 12 (GPIO18)      |
| D6           | 16 (GPIO23)      |
| D5           | 18 (GPIO24)      |
| D4           | 22 (GPIO25)      |
| D3           | Unused           |
| D2           | Unused           |
| D1           | Unused           |
| D0           | Unused           |
| E            | 15 (GPIO22)      |
| R/W          | GND              |
| RS           | 13 (GPIO27)      |
| OV           | Potentiometer    |
| VDD          | 5V               |
| VSS          | GND              |

