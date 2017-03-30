int heating_total_power()
{
	String value = get_selection_valueCSV("number of heaters", section_lines[0], section_lines[1]);
	boolean heaters = atoi(value.c_str()) - 1;
	int j = 0;
	int total_power = 0;
	//define the arrays to hold the sections positions
	int heater[heaters];						//array with heaters power
	for (byte i = 0; i <= heaters; i++) {
		j = i + 1;
		value = "heating power " + String(i + 1);
		value = get_selection_valueCSV(value, section_lines[0], section_lines[1]);
		heater[i] = atoi(value.c_str());
		total_power = total_power + heater[i];
	}
	return total_power;
}