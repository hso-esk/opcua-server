/* sample comment */
/* This EDDL file 2 describes a sample field device */


VARIABLE phys_soft_desc_2
{
	LABEL		[phys_soft_desc_label];
	HELP		[phys_soft_desc_help];
	CLASS		CONTAINED;
	TYPE		ASCII (32)
	{
		DEFAULT_VALUE "TEST String";
	}
	HANDLING	READ;
}


MANUFACTURER	66,
DEVICE_TYPE		0x070E,
DEVICE_REVISION 1,
DD_REVISION		1

/* passed */
VARIABLE trans1_static_press_value_2
{
	LABEL		trans1_static_press_value_label;
	HELP		trans1_static_press_value_help;
	CLASS		CONTAINED & DYNAMIC;
	TYPE		FLOAT;
	HANDLING	READ;
}


UNIT TB_STATIC_PRESSURE_VALUE_UNIT
{
	trans1_static_press_unit:
	
	trans1_static_press_value,
	func2_AI_pv_upper_range_value,
	func2_AI_pv_lower_range_value,
	func2_AI_simulate_value
}

VARIABLE phys_set_address_2
{
	LABEL		[set_address];
	HELP		[set_address_help];
	CLASS		CONTAINED & DYNAMIC;
	TYPE		UNSIGNED_INTEGER (1)
	{
		DEFAULT_VALUE 12;
	}
	HANDLING	READ & WRITE;
}

COMMAND write_trans1_display_cycle
{
	BLOCK transducer_block_1;
	INDEX 67;
	OPERATION WRITE;
	TRANSACTION
	{
		REQUEST
		{
			trans1_display_cycle
		}
		REPLY
		{
		}
	}
}
