//function to calculate approximate time in hours for heating (volume in liters, temperature gap in ?C or K, used average heating power)
//note when you replace heat_power with time in hours, you get approximate heating power to heat the volume in this time
unsigned long calc_aprox_time(int vol, float deltaT, int heat_power) {
	int n = 80;									//coefficient of efficiency (0-100), please change when the time is not accurate
												//float time_in_hours;
	unsigned long time_in_millis;
	unsigned long tmp = deltaT * 100;
	//time_in_hours = ((float)vol*1.163*deltaT)/((float)heat_power*n/100);
	time_in_millis = ((unsigned long)vol * 1163 * tmp * 36) / ((unsigned long)heat_power*n / 100);
	return time_in_millis;
}
