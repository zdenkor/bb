void proceed()
{
	lcd.clear();
	LCD_print_message(0, 7, 0);				//lcd.setCursor(0, 0); lcd.print(F("Are you sure, that"));
	LCD_print_message(1, 8, 0);				//lcd.setCursor(0, 1); lcd.print(F("you want to stop"));
	LCD_print_message(2, 9, 0);				//lcd.setCursor(0, 2); lcd.print(F("everything?"));
	LCD_print_message(3, 10, 0);			//lcd.setCursor(0, 3); lcd.print(F("[2]Y [3]N"));
	while (1)
	{
		analog_keypad.update();
		delay(50);
		if (analog_keypad.hasChanged())
		{
			// read the input on analog pin
			int sensorValue = analog_keypad.getValue();
			if (key(sensorValue) == 2)
			{
				lcd.clear();
				LCD_print_message(0, 11, 0);				//lcd.setCursor(0, 0); lcd.print(F("PLEASE, TURN ME OFF"));
				while (1);
			}
			if (key(sensorValue) == 3) return;
		}
	}
}