#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "AirConditioner.h"
#include <IRremote.h>

/* ======================== OBJECT INSTANTIATION ========================*/

/* IR receiver object instantiated with the specified pin */
IRrecv receiver(PIN_RECEIVER);

/* LiquidCrystal_I2C object instantiated with the specified address, number of columns, and number of rows */
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

/* DHT sensor object instantiated with the specified pin, type, and count */
DHT dht(DHT22_PIN, DHT22_TYPE, DTH22_COUNT);

/* Static initialization of the air conditioner state structure with default values */
static AirConditionerState AC_State = 
{
    .powerON                = OFF,                     /* Power state set to OFF */
    .set_temperature        = 0.0f,                    /* Default desired temperature */
    .measured_temperature   = 0.0f,                    /* Initial measured temperature */
    .set_humidity           = 0.0f,                    /* Default desired humidity */
    .measured_humidity      = 0.0f,                    /* Initial measured humidity */
    .enable_mode_change     = OFF,                     /* Mode change disabled by default */
    .mode                   = IDLE,                    /* Initial mode set to IDLE */
    .ventilation_mode       = VENTILATION_OFF          /* Ventilation mode set to OFF */
};


/* ======================== REMOTE CONTROL ========================*/
static void RemoteControlHandlerIR(void);

/* ======================== LCD ========================*/
static void LcdPrintModeOptions(void);
static void LcdPower(AirConditionerState * AC_State);
static void LcdPrint(AirConditionerState * AC_State);
static void LcdPrintTemperature(AirConditionerState * AC_State);
static void LcdPrintHumidity(AirConditionerState * AC_State);
static void LcdPrintVentilation(AirConditionerState * AC_State);
static void LcdPrintNormal(AirConditionerState * AC_State);
static void LcdPrintTempDiffHandle(AirConditionerState * AC_State);
static void LcdPrintHumidityDiffHandle(AirConditionerState * AC_State);

/* ======================== AIR CONDITIONER ========================*/
static void PrintAirConditionerState(AirConditionerState * AC_State);
static void AirConditionerTogglePower(AirConditionerState * AC_state);
static AirConditionerMode CheckAirConditionerMode(AirConditionerState * AC_State);
static void SetAirConditionerMode(AirConditionerState * AC_State, AirConditionerMode next_mode);
static void SetParamBasedOnMode(AirConditionerState * AC_State, ParamState param_state);
static void AirConditionerMain(AirConditionerState * AC_State);
static const char * GetAirConditionerMode(AirConditionerState * AC_State);
static void Init_Initial_AC_Parameters(void);

/* ======================== TEMPERATURE ========================*/
static void TempControlMain(AirConditionerState * AC_State);
static void Heating(AirConditionerState * AC_State, float temp_tick_rate);
static void Cooling(AirConditionerState * AC_State, float temp_tick_rate);
static float CalculateTemperatureDifference(AirConditionerState * AC_State);
static float CalculateTemperatureTickRate(float temp_difference);

/* ======================== HUMIDITY ========================*/
static void HumidityControlMain(AirConditionerState * AC_State);
static void Humidification(AirConditionerState * AC_State, float humidity_tick_rate);
static void Dehumidification(AirConditionerState * AC_State, float humidity_tick_rate);
static float CalculateHumidityDifference(AirConditionerState * AC_State);
static float CalculateHumidityTickRate(float humidity_difference);

/* ======================== VENTILATION ========================*/
static void SetVentilationMode(AirConditionerState * AC_State, ParamState param_state);
static void VentilationControlMain(AirConditionerState * AC_State);
static const char * GetVentilationFanPowerMode(AirConditionerState * AC_State);


void setup() 
{
  (void)Serial.begin(115200);
  (void)Serial.println("Starting serial!");
  lcd.init();
  (void)Serial.println("LCD was initialized successfully!");
  dht.begin();
  (void)Serial.println("DHT22 temperature and humidity sensor was initialized successfully!");
  receiver.enableIRIn();
  (void)Serial.println("IR Receiver was initialized!");
  Init_Initial_AC_Parameters();
  (void)Serial.println("Air Conditioner environment parameters were initialized successfully!");
}


void loop() 
{
  if ( receiver.decode() ) 
  {
    RemoteControlHandlerIR();
    receiver.resume();
  }
  else
  {
    /* Do nothing */
  }
}

/* ======================== REMOTE CONTROL ========================*/

static void RemoteControlHandlerIR(void)
{
  /* Takes command based on IR code received */ 
  switch (receiver.decodedIRData.command) 
  {
    case POWER:
      AirConditionerTogglePower(&AC_State);
      LcdPower(&AC_State);
      break;
    case MODE:
      LcdPrintModeOptions();
      AC_State.enable_mode_change = ON;
      break;
    case SET_PARAM_INCREASE:
      SetParamBasedOnMode(&AC_State, INCREASE);
      break;
    case PREVIOUS_FAN_POWER_MODE:
      SetVentilationMode(&AC_State, DECREASE);
      break;
    case RUN:
      AirConditionerMain(&AC_State);
      break;
    case NEXT_FAN_POWER_MODE:
      SetVentilationMode(&AC_State, INCREASE);
      break;
    case MODE_0_IDLE:
      SetAirConditionerMode(&AC_State, IDLE);
      break;
    case SET_PARAM_DECREASE:
      SetParamBasedOnMode(&AC_State, DECREASE);
      break;
    case MODE_1_COOLING:
      SetAirConditionerMode(&AC_State, COOLING);
      break;
    case MODE_2_HEATING:
      SetAirConditionerMode(&AC_State, HEATING);
      break;
    case MODE_3_VENTILATION:
      SetAirConditionerMode(&AC_State, VENTILATION);
      break;
    case MODE_4_HUMIDIFICATION:
      SetAirConditionerMode(&AC_State, HUMIDIFICATION);
      break;
    case MODE_5_DEHUMIDIFICATION:
      SetAirConditionerMode(&AC_State, DEHUMIDIFICATION);
      break;
    default:
      lcd.clear();
      break;
  }

  if ( ON == AC_State.powerON )
  {
    LcdPrint(&AC_State);
  }
  else
  {
    /* Do nothing */
  }
}


/* ======================== LCD ========================*/

static void LcdPrintModeOptions(void)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter mode:");
  lcd.setCursor(0, 1);
  lcd.print("0 - IDLE");
  lcd.setCursor(0, 2);
  lcd.print("1 - COOLING");
  lcd.setCursor(0, 3);
  lcd.print("2 - HEATING");
  delay(2 * DELAY_TIME);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter mode:");
  lcd.setCursor(0, 1);
  lcd.print("3 - VENTILATION");
  lcd.setCursor(0, 2);
  lcd.print("4 - HUMIDIFICATION");
  lcd.setCursor(0, 3);
  lcd.print("5 - DEHUMIDIFICATION");
  delay(2 * DELAY_TIME);
  lcd.clear();
}


static void LcdPower(AirConditionerState * AC_State)
{
  if ( ON == AC_State->powerON )
  {
    lcd.backlight();
  }
  else if ( OFF == AC_State->powerON )
  {
    lcd.clear();
    lcd.noBacklight();
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrint(AirConditionerState * AC_State)
{
  if ( NULL != AC_State)
  {
    if ( ( COOLING == AC_State->mode ) || ( HEATING == AC_State->mode ) )
    {
      LcdPrintTemperature(AC_State);
    }
    else if ( ( DEHUMIDIFICATION == AC_State->mode ) || ( HUMIDIFICATION == AC_State->mode ))
    {
      LcdPrintHumidity(AC_State);
    }
    else if ( VENTILATION == AC_State->mode )
    {
      LcdPrintVentilation(AC_State);
    }
    else
    {
      LcdPrintNormal(AC_State);
    }
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrintTemperature(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Conditioner");
    lcd.setCursor(0, 1);
    lcd.print("Mode ");
    lcd.println(GetAirConditionerMode(AC_State));
    lcd.setCursor(0, 2);
    lcd.print("Temperature: ");
    lcd.println(AC_State->measured_temperature);
    lcd.setCursor(0, 3);
    lcd.print("Set value: ");
    lcd.println(AC_State->set_temperature);
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrintHumidity(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Conditioner");
    lcd.setCursor(0, 1);
    lcd.print("Mode ");
    lcd.println(GetAirConditionerMode(AC_State));
    lcd.setCursor(0, 2);
    lcd.print("Humidity: ");
    lcd.println(AC_State->measured_humidity);
    lcd.setCursor(0, 3);
    lcd.print("Set value: ");
    lcd.println(AC_State->set_humidity);
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrintVentilation(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Conditioner");
    lcd.setCursor(0, 1);
    lcd.print("Mode: ");
    lcd.println(GetAirConditionerMode(AC_State));
    lcd.setCursor(0, 2);
    lcd.print("Fan Power: ");
    lcd.println(GetVentilationFanPowerMode(AC_State));
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrintNormal(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Conditioner");
    lcd.setCursor(0, 1);
    lcd.print("Mode ");
    lcd.println(GetAirConditionerMode(AC_State));
    lcd.setCursor(0, 2);
    lcd.print("Temperature: ");
    lcd.println(AC_State->measured_temperature);
    lcd.setCursor(0, 3);
    lcd.print("Humidity: ");
    lcd.println(AC_State->measured_humidity);
  }
  else
  {
    /* Do nothing */
  }
}


static void LcdPrintTempDiffHandle(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("WARNING!");
    lcd.setCursor(0, 1);
    lcd.print("Set temp value");
    lcd.setCursor(0, 2);
    lcd.print("incorrect");
    lcd.setCursor(0, 3);
    lcd.print("Entering IDLE state");
  }
  else
  {
    /* Do nothing */
  }
  delay(DELAY_TIME);
}


static void LcdPrintHumidityDiffHandle(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("WARNING!");
    lcd.setCursor(0, 1);
    lcd.print("Set humidity value");
    lcd.setCursor(0, 2);
    lcd.print("incorrect");
    lcd.setCursor(0, 3);
    lcd.print("Entering IDLE state");
  }
  else
  {
    /* Do nothing */
  }
  delay(DELAY_TIME);
}

/* ======================== AIR CONDITIONER ========================*/

static void PrintAirConditionerState(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    (void)Serial.print("=============================\n");
    (void)Serial.print("Power ON: ");
    (void)Serial.println(AC_State->powerON ? "ON" : "OFF");
    (void)Serial.print("Set Temperature: ");
    (void)Serial.println(AC_State->set_temperature);
    (void)Serial.print("Measured Temperature: ");
    (void)Serial.println(AC_State->measured_temperature);
    (void)Serial.print("Set Humidity: ");
    (void)Serial.println(AC_State->set_humidity);
    (void)Serial.print("Measured Humidity: ");
    (void)Serial.println(AC_State->measured_humidity);
    (void)Serial.print("Mode: ");
    switch (AC_State->mode)
    {
      case IDLE:
        (void)Serial.println("IDLE");
        break;
      case COOLING:
        (void)Serial.println("COOLING");
        break;
      case HEATING:
        (void)Serial.println("HEATING");
        break;
      case VENTILATION:
        (void)Serial.println("VENTILATION");
        break;
      case HUMIDIFICATION:
        (void)Serial.println("HUMIDIFICATION");
        break;
      case DEHUMIDIFICATION:
        (void)Serial.println("DEHUMIDIFICATION");
        break;
      default:
        (void)Serial.println("IDLE");
        break;
    }
    (void)Serial.print("=============================\n");
  }
  else
  {
    /* Do nothing */
  }
}


static void AirConditionerTogglePower(AirConditionerState * AC_state)
{
  switch ( AC_state->powerON )
  {
    case OFF:
      AC_state->powerON = ON;
      break;
    case ON:
      AC_state->powerON = OFF;
      break;
    default:
      AC_state->powerON = OFF;
      break;
  }
}


static AirConditionerMode CheckAirConditionerMode(AirConditionerState * AC_State)
{
  return AC_State->mode;
}


static void SetAirConditionerMode(AirConditionerState * AC_State, AirConditionerMode next_mode)
{
  if ( ( next_mode != AC_State->mode ) && ( ON == AC_State->enable_mode_change ) )
  {
    AC_State->mode = next_mode;
  }
  else
  {
    /* Do nothing */
  }
  AC_State->enable_mode_change = OFF;
}


static void SetParamBasedOnMode(AirConditionerState * AC_State, ParamState param_state)
{
  if ( COOLING == AC_State->mode )
  {
    if ( DECREASE == param_state )
    {
      AC_State->set_temperature -= 0.1f;
    }
    else
    {
      AC_State->set_temperature += 0.1f;
    }
  }
  else if ( HEATING == AC_State->mode )
  {
    if ( DECREASE == param_state )
    {
      AC_State->set_temperature -= 0.1f;
    }
    else
    {
      AC_State->set_temperature += 0.1f;
    }
  }
  else if ( DEHUMIDIFICATION == AC_State->mode )
  {
    if ( DECREASE == param_state )
    {
      AC_State->set_humidity -= 0.5f;
    }
    else
    {
      AC_State->set_humidity += 0.5f;
    }
  }
  else if ( HUMIDIFICATION == AC_State->mode )
  {
    if ( DECREASE == param_state )
    {
      AC_State->set_humidity -= 0.5f;
    }
    else
    {
      AC_State->set_humidity += 0.5f;
    }
  }
  else
  {
    /* Do nothing */
  }
}


static void AirConditionerMain(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    if ( ( DEHUMIDIFICATION == AC_State->mode ) || ( HUMIDIFICATION == AC_State->mode ) )
    {
      HumidityControlMain(AC_State);
    }
    else if ( ( COOLING == AC_State->mode ) || ( HEATING == AC_State->mode ) )
    {
      TempControlMain(AC_State);
    }
    else if ( VENTILATION == AC_State->mode )
    {
      VentilationControlMain(AC_State);
    }
    else
    {
      /* Do nothing */
    }
  }
  else
  {
    /* Do nothing */
  }
}


static const char * GetAirConditionerMode(AirConditionerState * AC_State)
{
  const char * AC_mode_str = "IDLE";
  switch ( AC_State->mode )
  {
    case IDLE:
      AC_mode_str = "IDLE";
      break;
    case COOLING:
      AC_mode_str = "COOLING";
      break;
    case HEATING:
      AC_mode_str = "HEATING";
      break;
    case VENTILATION:
      AC_mode_str = "VENTILATION";
      break;
    case HUMIDIFICATION:
      AC_mode_str = "HUMIDIFICATION";
      break;
    case DEHUMIDIFICATION:
      AC_mode_str = "DEHUMIDIFICATION";
      break;
    default:
      AC_mode_str = "IDLE";
      break;
  }
  return AC_mode_str;
}

static void Init_Initial_AC_Parameters(void)
{
  AC_State.measured_temperature = dht.readTemperature();
  AC_State.set_temperature = AC_State.measured_temperature;
  AC_State.measured_humidity = dht.readHumidity();
  AC_State.set_humidity = AC_State.measured_humidity;
}


/* ======================== TEMPERATURE ========================*/

static void Heating(AirConditionerState * AC_State, float temp_tick_rate)
{
  if ( NULL != AC_State )
  {
    AC_State->measured_temperature += temp_tick_rate;
  }
  else
  {
    /* Do nothing */
  }
}


static void Cooling(AirConditionerState * AC_State, float temp_tick_rate)
{
  if ( NULL != AC_State )
  {
    AC_State->measured_temperature -= temp_tick_rate;
  }
  else
  {
    /* Do nothing */
  }
}


static float CalculateTemperatureDifference(AirConditionerState * AC_State)
{
  float temp_difference = 0.0f;
  if ( NULL != AC_State )
  {
    if ( HEATING == AC_State->mode )
    {
      temp_difference = AC_State->set_temperature - AC_State->measured_temperature;
      if ( temp_difference < 0.0f )
      {
        temp_difference = 0.0f;
        AC_State->mode = IDLE;
        AC_State->set_temperature = AC_State->measured_temperature;
        LcdPrintTempDiffHandle(AC_State);
        delay(DELAY_TIME);
      }
      else
      {
        /* Do nothing */
      }

    }
    else if ( COOLING == AC_State->mode )
    {
      temp_difference = AC_State->set_temperature - AC_State->measured_temperature;
      if ( temp_difference > 0.0f )
      {
        temp_difference = 0.0f;
        AC_State->mode = IDLE;
        AC_State->set_temperature = AC_State->measured_temperature;
        LcdPrintTempDiffHandle(AC_State);
        delay(DELAY_TIME);
      }
      else
      {
        /* Do nothing */
      }
    }
    else
    {
      /* Do nothing */
    }
  }
  else
  {
    /* Do nothing */
  }
  return abs(temp_difference);
}


static float CalculateTemperatureTickRate(float temp_difference)
{
  float temp_tick_rate = 0.0f;
  if ( TEMP_BOUND_HIGH <= temp_difference )
  {
    temp_tick_rate = 0.5f;
  }
  else if ( ( TEMP_BOUND_HIGH > temp_difference ) && ( temp_difference >= TEMP_BOUND_MEDIUM ) )
  {
    temp_tick_rate = 0.3f;
  }
  else if ( ( TEMP_BOUND_MEDIUM > temp_difference ) && ( temp_difference >= TEMP_BOUND_LOW ) )
  {
    temp_tick_rate = 0.2f;
  }
  else
  {
    temp_tick_rate = 0.1f;
  }
  return temp_tick_rate;
}


static void TempControlMain(AirConditionerState * AC_State)
{
  if (NULL != AC_State)
  {
    float temp_difference = CalculateTemperatureDifference(AC_State);
    while ( TEMPERATURE_TOLERANCE <= temp_difference )
    {
      delay(DELAY_TIME);
      float temp_tick_rate = CalculateTemperatureTickRate(temp_difference);
      if ( HEATING == AC_State->mode )
      {
        Heating(AC_State, temp_tick_rate);
      }
      else if ( COOLING == AC_State->mode )
      {
        Cooling(AC_State, temp_tick_rate);
      }
      temp_difference = CalculateTemperatureDifference(AC_State);
      LcdPrint(AC_State);
      PrintAirConditionerState(AC_State);
    }
    delay(DELAY_TIME);
    AC_State->mode = IDLE;
  }
  else
  {
    /* Do nothing */
  }
}


/* ======================== HUMIDITY ========================*/

static void Humidification(AirConditionerState * AC_State, float humidity_tick_rate)
{
  if ( NULL != AC_State )
  {
    AC_State->measured_humidity += humidity_tick_rate;
  }
  else
  {
    /* Do nothing */
  }
}

static void Dehumidification(AirConditionerState * AC_State, float humidity_tick_rate)
{
  if ( NULL != AC_State )
  {
    AC_State->measured_humidity -= humidity_tick_rate;
  }
  else
  {
    /* Do nothing */
  }
}


static float CalculateHumidityDifference(AirConditionerState * AC_State)
{
  float humidity_difference = 0.0f;
  if ( NULL != AC_State )
  {
    if ( HUMIDIFICATION == AC_State->mode )
    {
      humidity_difference = AC_State->set_humidity - AC_State->measured_humidity;
      if ( humidity_difference < 0.0f )
      {
        humidity_difference = 0.0f;
        AC_State->mode = IDLE;
        AC_State->set_humidity = AC_State->measured_humidity;
        LcdPrintHumidityDiffHandle(AC_State);
      }
      else
      {
        /* Do nothing */
      }

    }
    else if ( DEHUMIDIFICATION == AC_State->mode )
    {
      humidity_difference = AC_State->set_humidity - AC_State->measured_humidity;
      if ( humidity_difference > 0.0f )
      {
        humidity_difference = 0.0f;
        AC_State->mode = IDLE;
        AC_State->set_humidity = AC_State->measured_humidity;
        LcdPrintHumidityDiffHandle(AC_State);
      }
      else
      {
        /* Do nothing */
      }
    }
    else
    {
      /* Do nothing */
    }
  }
  else
  {
    /* Do nothing */
  }

  return abs(humidity_difference);
}


static float CalculateHumidityTickRate(float humidity_difference)
{
  float humidity_tick_rate = 0.0f;
  if ( HUMIDITY_BOUND_HIGH <= humidity_difference )
  {
    humidity_tick_rate = 0.5f;
  }
  else if ( ( HUMIDITY_BOUND_HIGH > humidity_difference ) && ( humidity_difference >= HUMIDITY_BOUND_MEDIUM ) )
  {
    humidity_tick_rate = 0.3f;
  }
  else if ( ( HUMIDITY_BOUND_MEDIUM > humidity_difference ) && ( humidity_difference >= HUMIDITY_BOUND_LOW ) )
  {
    humidity_tick_rate = 0.2f;
  }
  else
  {
    humidity_tick_rate = 0.1f;
  }
  return humidity_tick_rate;
}


static void HumidityControlMain(AirConditionerState * AC_State)
{
  if (NULL != AC_State)
  {
    float humidity_difference = CalculateHumidityDifference(AC_State);
    while ( HUMIDITY_TOLERANCE <= humidity_difference )
    {
      delay(DELAY_TIME);
      float humidity_tick_rate = CalculateHumidityTickRate(humidity_difference);
      if ( HUMIDIFICATION == AC_State->mode )
      {
        Humidification(AC_State, humidity_tick_rate);
      }
      else if ( DEHUMIDIFICATION == AC_State->mode )
      {
        Dehumidification(AC_State, humidity_tick_rate);
      }
      else
      {
        /* Do nothing */
      }
      humidity_difference = CalculateHumidityDifference(AC_State);
      LcdPrint(AC_State);
      PrintAirConditionerState(AC_State);
    }
    delay(DELAY_TIME);
    AC_State->mode = IDLE;
  }
  else
  {
    /* Do nothing */
  }
}


/* ======================== VENTILATION ========================*/

static void SetVentilationMode(AirConditionerState * AC_State, ParamState param_state)
{
  if ( (NULL != AC_State ) && ( VENTILATION == AC_State->mode ) )
  {
    switch ( AC_State->ventilation_mode )
    {
      case VENTILATION_OFF:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_20_PERCENT : VENTILATION_OFF;
        break;
      case VENTILATION_20_PERCENT:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_40_PERCENT : VENTILATION_OFF;
        break;
      case VENTILATION_40_PERCENT:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_60_PERCENT : VENTILATION_20_PERCENT;
        break;
      case VENTILATION_60_PERCENT:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_80_PERCENT : VENTILATION_40_PERCENT;
        break;
      case VENTILATION_80_PERCENT:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_100_PERCENT : VENTILATION_60_PERCENT;
        break;
      case VENTILATION_100_PERCENT:
        AC_State->ventilation_mode = (INCREASE == param_state) ? VENTILATION_100_PERCENT : VENTILATION_80_PERCENT;
        break;
      default:
        AC_State->ventilation_mode = VENTILATION_OFF;
        break;
    }
  }
  else
  {
    /* Do nothing */
  }
}

static void VentilationControlMain(AirConditionerState * AC_State)
{
  if ( NULL != AC_State )
  {
    delay(DELAY_TIME);
    LcdPrint(AC_State);
  }
  else
  {
    /* Do nothing */
  }
}

const char * GetVentilationFanPowerMode(AirConditionerState * AC_State) {
  const char * fan_power_str = "Off";

  switch ( AC_State->ventilation_mode ) {
    case VENTILATION_OFF:
      fan_power_str = "Off";
      break;
    case VENTILATION_20_PERCENT:
      fan_power_str = "20 %";
      break;
    case VENTILATION_40_PERCENT:
      fan_power_str = "40 %";
      break;
    case VENTILATION_60_PERCENT:
      fan_power_str = "60 %";
      break;
    case VENTILATION_80_PERCENT:
      fan_power_str = "80 %";
      break;
    case VENTILATION_100_PERCENT:
      fan_power_str = "100 %";
      break;
    default:
      fan_power_str = "Off";
      break;
  }
  return fan_power_str;
}





