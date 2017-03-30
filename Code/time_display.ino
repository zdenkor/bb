//functions to print time
void printDigits(int digits) {
	lcd.print(":");
	if (digits < 10)
		lcd.print('0');
	lcd.print(digits);
}

void TimeDisplay(unsigned long starttime, unsigned long actualtime) {
	int h, m, s;
	s = (actualtime - starttime) / 1000;
	m = s / 60;
	h = s / 3600;
	s = s - m * 60;
	m = m - h * 60;
	lcd.print(h);
	printDigits(m);
	printDigits(s);
}