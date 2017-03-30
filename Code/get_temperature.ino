float getTemperature() {
	float analog_value;							//define average value

												// update the ResponsiveAnalogRead object every loop
	analog_temperature.update();
	analog_value = analog_temperature.getValue();

	// convert the value to resistance
	analog_value = 1023 / analog_value - 1;
	analog_value = SERIESRESISTOR / analog_value;

	// calculate the temperature
	float steinhart;
	steinhart = analog_value / THERMISTORNOMINAL;		// (R/Ro)
	steinhart = log(steinhart);							// ln(R/Ro)
	steinhart /= BCOEFFICIENT;							// 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);	// + (1/To)
	steinhart = 1.0 / steinhart;						// Invert
	steinhart -= 273.15;								// convert to ?C

	return(steinhart + CALIBRATION);
}
