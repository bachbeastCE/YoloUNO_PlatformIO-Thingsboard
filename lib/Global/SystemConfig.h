#ifndef SystemConfig_h
#define SystemConfig_h

/*Number of location*/
#define LOCATION_NUMBER 1

/// brief@ Define sensor type, button, led, etc...
#define DHT_TYPE 20
#define MAX_BUTTON 3

/// brief@ Define <sensor name> _ <sensor's pinout name> _ PIN
#define DHT11_SIGNAL_PIN GPIO_NUM_18
//#define MQ150_D1_PIN GPIO_NUM_11

#define SDA_PIN GPIO_NUM_11
#define SCL_PIN GPIO_NUM_12

#define LED1_PIN GPIO_NUM_3
#define LED2_PIN GPIO_NUM_4
#define FAN_PIN GPIO_NUM_8

#define button1 GPIO_NUM_9
#define button2 GPIO_NUM_10
//#define button3 GPIO_NUM_19
//#define button4 GPIO_NUM_26


#endif