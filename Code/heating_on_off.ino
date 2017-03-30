//heating function
void heating_on_off(int i)
{
	//if heating ON
	if (i == ON)
	{
		digitalWrite(PIN_HEATER1, LOW);
		digitalWrite(PIN_HEATER2, LOW);
		digitalWrite(PIN_MIXING, LOW);
		heating_status = ON;
		mixing_status = ON;
	}
	//if heating OFF
	if (i == OFF)
	{
		digitalWrite(PIN_HEATER1, HIGH);
		digitalWrite(PIN_HEATER2, HIGH);
		digitalWrite(PIN_MIXING, HIGH);
		if (phase_heating != 1)
		{
			heating_status = OFF;
			lcd.setCursor(9, 3);
			lcd.print(" ");
		}
		if (phase_mixing != 1)
		{
			mixing_status = OFF;
			lcd.setCursor(11, 3);
			lcd.print(" ");
		}
		//timer pre mixing delay off definuj
		//timer_for_mixing_delay.restart();
	}
}
