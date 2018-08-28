/**
 * EDDL description file for a sample Current Sensor. Currently
 * only two variables are supported namely the actual current
 * and its unit.
 */


/*
 * Comon device description section.
 */

MANUFACTURER	66,
DEVICE_TYPE		0x070E,
DEVICE_REVISION 1,
DD_REVISION		1


/**
 * Variable for the actual current.
 */
VARIABLE var_curr_sens_curr
{
	LABEL		Current;
	HELP		Actual_current;
	CLASS		CONTAINED & DYNAMIC;
	TYPE		FLOAT;
	HANDLING	READ;
}

/**
 * Variable for the temperature unit.
 */
VARIABLE var_curr_sens_unit
{
	LABEL		Unit;
	HELP		Describes_the_unit_of_the_current_sensor_value;
	CLASS		CONTAINED;
	TYPE		ASCII (32)
	{
		DEFAULT_VALUE "mA";
	}
	HANDLING	READ;
}

