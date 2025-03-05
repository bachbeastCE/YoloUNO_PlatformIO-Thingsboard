#ifndef button_h
#define button_h

#include "Arduino.h"
#include "SystemConfig.h"

#define NORMAL_STATE SET
#define PRESSED_STATE RESET

extern int button_flag[5];

#define SET 1
#define RESET 0
#define TICK 50

void getKeyInput();
int isButtonPressed(int button_index);
int isButtonLongPressed(int button_index);

#endif /* INC_BUTTON_H_ */