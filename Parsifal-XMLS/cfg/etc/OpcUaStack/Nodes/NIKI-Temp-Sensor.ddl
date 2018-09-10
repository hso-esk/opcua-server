/**
 * EDDL description file for a sample NIKI Temperature Sensor. Currently
 * only two variables are supported namely the current temperature
 * and its unit.
 */


/*
 * Comon device description section.
 */

MANUFACTURER	55,
DEVICE_TYPE		0x070D,
DEVICE_REVISION 1,
DD_REVISION		1


/**
 * Variable for the current temperature.
 */
VARIABLE var_temp_sens_temp
{
	LABEL		Temperature;
	HELP		Current_temperature;
	CLASS		CONTAINED & DYNAMIC;
	TYPE		FLOAT;
	HANDLING	READ;
}

/**
 * Variable for the temperature unit.
 */
VARIABLE var_temp_sens_unit
{
	LABEL		Unit;
	HELP		Describes_the_unit_of_the_temperature_sensor_value;
	CLASS		CONTAINED;
	TYPE		ASCII (32)
	{
		DEFAULT_VALUE "°C";
	}
	HANDLING	READ;
}

