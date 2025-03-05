#ifndef global_h
#define global_h

#include <ThingsBoard.h>
#include <string.h>

extern float temperature;
extern float humidity;
extern float light_intensity;
extern float air_quality;

extern ThingsBoard tb;

extern bool led1_state;
extern bool led2_state;
extern int fan_state;
#endif // GLOBAL_HPP