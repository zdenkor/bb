void phase_end(String phase_message)
{
	actual_phase = actual_phase + 1;
	timer_for_phase.stop();
	heating_on_off(OFF);
	//if manual continue of the next phase is required
	if (phase_autosteping == 0)
	{
		LCD_print_message(1, 5, 0);				//lcd.setCursor(0, 1); lcd.print(F(" phase has finished "));
		lcd.setCursor(0, 2);
		lcd.print(phase_message);
		LCD_print_message(3, 6, 1);				//lcd.setCursor(0, 3); lcd.print(F("[1]Y  Continue? [4]N"));
		while (1)
		{
			if (timer_for_alarm.onRestart())
			{
				melody_end_phase();
				timer_for_alarm.restart();
			}
			analog_keypad.update();
			delay(50);
			if (analog_keypad.hasChanged())
			{
				// read the input on analog pin
				int sensorValue = analog_keypad.getValue();
				if (key(sensorValue) == 1)
				{
					break;
				}
				if (key(sensorValue) == 4)
				{
					proceed();								//ask if you are sure
					break;
				}
			}
		}
	}
	return;
}

