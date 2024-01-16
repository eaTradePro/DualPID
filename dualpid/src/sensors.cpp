#include "sensors.h"
#include "config.h"

// #include "tools-log.h"

const char*     Temperature_Name                        = "Temperature";
const char*     Temperature_Unit_Text                   = "\xc2\xb0""C";
const double    Temperature_Setpoint_Min                = -20;
const double    Temperature_Setpoint_Max                = 60;
const int       Temperature_Precision                   = 2;
const lv_color_t Temperature_Color                      = COLOR_RED;

const char*     Humidity_Name                           = "Humidity";
const char*     Humidity_Unit_Text                      = "%RH";
const double    Humidity_Setpoint_Min                   = 0;
const double    Humidity_Setpoint_Max                   = 100;
const int       Humidity_Precision                      = 0;
const lv_color_t Humidity_Color                         = COLOR_BLUE;

const char*     CO2_Name                                = "CO2 Content";
const char*     CO2_Unit_Text                           = "%";
const double    CO2_Setpoint_Min                        = 0;
const double    CO2_Setpoint_Max                        = 20;
const int       CO2_Precision                           = 3;
const lv_color_t CO2_Color                      		= COLOR_YELLOW;

sensor_begin_fptr find_sensor_begin(uint32_t sensor_type)
{
	switch(sensor_type)
	{
		case SENSOR_SHT31_TEMP:		return sensor_sht31_begin;
		case SENSOR_M5KMETER:		return sensor_m5kmeter_begin;
		case SENSOR_MCP9600:		return sensor_mcp9600_begin;
		case SENSOR_MAX31865:		return sensor_max31865_begin;
		case SENSOR_SHT31_RH:		return sensor_sht31_begin;
		case SENSOR_SPRINTIR:		return sensor_sprintir_begin;
        default: return nullptr;
	};
};

sensor_read_fptr find_sensor_read(uint32_t sensor_type)
{
	switch(sensor_type)
	{
		case SENSOR_SHT31_TEMP:		return sensor_sht31temp_read;
		case SENSOR_M5KMETER:		return sensor_m5kmeter_read;
		case SENSOR_MCP9600:		return sensor_mcp9600_read;
		case SENSOR_MAX31865:		return sensor_max31865_read;
		case SENSOR_SHT31_RH:		return sensor_sht31rh_read;
		case SENSOR_SPRINTIR:		return sensor_sprintir_read;
        default: return nullptr;
	};
};

lv_color_t find_sensor_color(uint32_t sensor_type)
{
	switch(sensor_type)
	{
		case 100 ... 199: // Temperature
			return Temperature_Color;
		case 200 ... 299: // Humidity
			return Humidity_Color;
		case 300 ... 399: // CO2
			return CO2_Color;
		case SENSOR_NONE: //break;
		default:
			return COLOR_BLACK;
	};    
};


#ifdef SENSOR_SHT31_ENABLED
#include <SHT3x.h>
SHT3X sensor_sht31(Wire);\
bool sensor_sht31_begin()
{
    if(!sensor_sht31.begin(SHT3X_ADDRESS_DEFAULT))
        if(!sensor_sht31.begin(SHT3X_ADDRESS_ALT))
            return false;
    return true;
};
double sensor_sht31temp_read()
{
    SHT3X::measurement_t* m = sensor_sht31.newMeasurement();

    if(m->error)
        return NAN;
    return m->temperature;    
};
double sensor_sht31rh_read()
{
    SHT3X::measurement_t* m = sensor_sht31.newMeasurement();

    if(m->error)
        return NAN;
    return m->humidity;
};
#endif

#ifdef SENSOR_M5KMETER_ENABLED
#include <M5_KMeter.h>
M5_KMeter sensor_kmeter;
bool sensor_m5kmeter_begin()
{
    sensor_kmeter.begin(&Wire);
    return sensor_kmeter.setSleepTime(1);
};
double sensor_m5kmeter_read()
{
    if(!sensor_kmeter.update())
        return NAN;
    return sensor_kmeter.getTemperature();
};
#endif

#ifdef SENSOR_MCP9600_ENABLED
#include <MCP9600.h>
MCP9600 mcp9600;
bool sensor_mcp9600_begin()
{
    return mcp9600.begin();
};
double sensor_mcp9600_read()
{
    return mcp9600.readThermocouple();
};
#endif

#ifdef SENSOR_MAX31865_ENABLED
#include <MAX31865.h>
MAX31865 max31865(SPI, PIN_MAX31865_CS);
bool sensor_max31865_begin()
{
    if(max31865.begin(100, 430, true, true))
	{
		max31865.setAutoConvert(true);
		return true;
	};
	return false;
};
double sensor_max31865_read()
{
	time_t start = millis();
    return max31865.getTemperature();
};
#endif

#ifdef SENSOR_SPRINTIR_ENABLED
#include <SprintIR.h>
SprintIR sensor_sprintir(Serial2);

bool sensor_sprintir_begin()
{
	Serial2.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
    return sensor_sprintir.begin();
};
double sensor_sprintir_read()
{
	// CO2 from SprintIR-WX-20
	return sensor_sprintir.getPercent();
	// return sensor_sprintir.getCompensatedPercent(SPRINTIR_PRESSURE_MBAR);
};
#endif
