int key(int sensorValue)
{
	int key = 0;
	if (sensorValue >= 635 && sensorValue <= 715) key = 1;      // key 1
	else if (sensorValue >= 960 && sensorValue <= 1040) key = 2;  // key 2
	else if (sensorValue >= 395 && sensorValue <= 475) key = 3;   // key 3
	else if (sensorValue >= 485 && sensorValue <= 575) key = 4;   // key 4
	else key = 0;
	return key;
}