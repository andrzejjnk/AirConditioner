#ifndef AIRCONDITIONER_H
#define AIRCONDITIONER_H

#include <stdbool.h>


/* ======================== DEFINES ========================*/

/* LCD settings */
#define LCD_ADDRESS    ( 0x27U )   /* I2C address of the LCD */
#define LCD_COLS       ( 20U )     /* Number of columns on the LCD */
#define LCD_ROWS       ( 4U )      /* Number of rows on the LCD */

/* DHT22 sensor settings */
#define DHT22_PIN      ( 13U )     /* Digital pin connected to DHT22 sensor */
#define DHT22_TYPE     ( 22U )     /* Type of the DHT sensor */
#define DTH22_COUNT    ( 1U )      /* Number of DHT22 sensors */

/* IR Receiver settings */
#define PIN_RECEIVER   ( 12U )     /* Signal Pin of IR receiver */

/* Boolean states */
#define ON             ( true )    /* Boolean state for ON */
#define OFF            ( false )   /* Boolean state for OFF */

/* Null pointer */
#define NULL           ( (void *) 0) /* Null pointer definition */

/* Temperature bounds and tolerance */
#define TEMP_BOUND_HIGH         ( 3U )       /* High temperature bound */
#define TEMP_BOUND_MEDIUM       ( 1U )       /* Medium temperature bound */
#define TEMP_BOUND_LOW          ( (float)0.5 ) /* Low temperature bound */
#define TEMPERATURE_TOLERANCE   ( (float)0.1 ) /* Temperature tolerance */

/* Humidity bounds and tolerance */
#define HUMIDITY_BOUND_HIGH     ( 10U )      /* High humidity bound */
#define HUMIDITY_BOUND_MEDIUM   ( 5U )       /* Medium humidity bound */
#define HUMIDITY_BOUND_LOW      ( 2U )       /* Low humidity bound */
#define HUMIDITY_TOLERANCE      ( float(0.1) ) /* Humidity tolerance */

/* Delay time */
#define DELAY_TIME              ( 1000U )    /* Default delay time */


/* ======================== ENUMS ========================*/

/* Air conditioner modes */
typedef enum
{
  IDLE                 = 0,    /* Air conditioner in idle mode */
  COOLING              = 1,    /* Air conditioner in cooling mode */
  HEATING              = 2,    /* Air conditioner in heating mode */
  VENTILATION          = 3,    /* Air conditioner in ventilation mode */
  HUMIDIFICATION       = 4,    /* Air conditioner in humidification mode */
  DEHUMIDIFICATION     = 5     /* Air conditioner in dehumidification mode */
} AirConditionerMode;


/* Parameter state */
typedef enum
{
  DECREASE   = 0,   /* State for decreasing a parameter */
  INCREASE   = 1    /* State for increasing a parameter */
} ParamState;


/* Ventilation power modes */
typedef enum
{
  VENTILATION_OFF           = 0,    /* Ventilation turned off */
  VENTILATION_20_PERCENT    = 1,    /* Ventilation at 20% power */
  VENTILATION_40_PERCENT    = 2,    /* Ventilation at 40% power */
  VENTILATION_60_PERCENT    = 3,    /* Ventilation at 60% power */
  VENTILATION_80_PERCENT    = 4,    /* Ventilation at 80% power */
  VENTILATION_100_PERCENT   = 5     /* Ventilation at full power */
} VentilationPowerModes;


/* IR remote control commands */
typedef enum 
{
  POWER                      = 162,   /* Power button */
  MODE                       = 226,   /* Mode button */
  SET_PARAM_INCREASE         = 2,     /* Increase parameter button */
  PREVIOUS_FAN_POWER_MODE    = 224,   /* Previous fan power mode button */
  RUN                        = 168,   /* Run button */
  NEXT_FAN_POWER_MODE        = 144,   /* Next fan power mode button */
  MODE_0_IDLE                = 104,   /* Idle mode button */
  SET_PARAM_DECREASE         = 152,   /* Decrease parameter button */
  MODE_1_COOLING             = 48,    /* Cooling mode button */
  MODE_2_HEATING             = 24,    /* Heating mode button */
  MODE_3_VENTILATION         = 122,   /* Ventilation mode button */
  MODE_4_HUMIDIFICATION      = 16,    /* Humidification mode button */
  MODE_5_DEHUMIDIFICATION    = 56     /* Dehumidification mode button */
} IRCommand;


/* ======================== STRUCT ========================*/

/* Air conditioner state structure */
typedef struct state
{
  bool powerON;                 /* Indicates if the air conditioner is powered on */
  float set_temperature;        /* Desired temperature set by the user */
  float measured_temperature;   /* Current temperature measured by the air conditioner */
  float set_humidity;           /* Desired humidity set by the user */
  float measured_humidity;      /* Current humidity measured by the air conditioner */
  bool enable_mode_change;      /* Flag to enable mode change */
  AirConditionerMode mode;      /* Current mode of the air conditioner */
  VentilationPowerModes ventilation_mode;   /* Current ventilation power mode */
} AirConditionerState;



#endif //AIRCONDITIONER_H