#include <SdFat.h>						//SD library
#include <CSVFile.h>					//CSV parser for SDfat library
#include <Wire.h>						//signal controll library
#include <LiquidCrystal_I2C.h>			//library for LCD
#include <SPI.h>						//library for SPI (SD)
#include <stdlib.h>						//for some function, like float to string (dtostrf)
//#include <MemoryFree.h>					//for testing actual memory usage
#include <ResponsiveAnalogRead.h>		//library to avoid reading wrong values from analog ports
#include "pitches.h"					//for generating tones
#include <RBD_Timer.h>					//timer library https://github.com/alextaujenis/RBD_Timer

LiquidCrystal_I2C lcd(0x27, 20, 4);		// inicializacia LCD (adresa, znaky, riadky)

/* DEFINITION OF TIMERS*/
RBD::Timer timer_for_phase;				// timer for phase that has a exact time value
RBD::Timer timer_for_loop;				// timer for call function to displaying actual phase information
RBD::Timer timer_for_alarm;				// timer for call alarm
RBD::Timer timer_for_mixing_delay;		// timer for mixing off delay

#define PIN_SD_CS 10					//port for SD card initialization
//SPI speed, change this value if you have problems with SD card
// Available values: SPI_QUARTER_SPEED //SPI_HALF_SPEED
#define SD_CARD_SPEED SPI_FULL_SPEED	
// If you have connected other SPI device then
// put here number of pin for disable its.
// Provide -1 if you don't have other devices.
#define PIN_OTHER_DEVICE_CS -1

//sound
#define PIN_MELODY 3						//digital port to play the sound

#define PIN_KEYPAD A7						// analog pin to connect keypad devider
// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead analog_keypad(PIN_KEYPAD, false);

//relays
#define PIN_HEATER1 7						//digital port to control Heater #1
#define PIN_HEATER2 8						//digital port to control Heater #2
#define PIN_MIXING 9						//digital port to control mixing
#define ON 1								//relay on
#define OFF 0								//relay off

//temperature sensor NTC100k
#define PIN_THERMISTORPIN A0				// analog pin to connect thermistor devider
// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead analog_temperature(PIN_THERMISTORPIN, false);
#define THERMISTORNOMINAL 110000			// resistance at 25 degrees C default 100000
#define TEMPERATURENOMINAL 25			// temp. for nominal resistance (almost always 25 C)
#define BCOEFFICIENT 4000				// The beta coefficient of the thermistor (usually 3000-4000) default 3950
#define SERIESRESISTOR 100000			// the value of the 'other' resistor 
#define CALIBRATION 0					// for calibration measured temperature
#define TEMP_OFFSET 0.5					// allowed temp offset for temperature, so when to turn on/off heating
#define COOLING -1						// cooling phase
#define KEEPING 0						// keep temperature phase
#define HEATING 1						// heating phase

//SD CSV file
// Good practice is declare buffer size larger by one
// then we need and put as last null character.
// It is prevent from overbuffer.
const byte BUFFER_SIZE = 20;			//define size of buffer for reading CSV file (max number of characters in one field (LCD line lenght plus 1)
#define FILENAME "beerbrew.csv"			//the name of ini file
SdFat sd;								//definition for sd card
CSVFile csv;							//define CSF file

//other global variables
char buffer[BUFFER_SIZE + 1];			//setting the buffer
int phases = 0;							//number of all phases
int actual_phase = 0;					//define actual phase number, need to be const due to Inifile library
int latest_phase = 0;					//define latest phase to compare if you can go to next
boolean phase_autosteping;				//0 - OFF, required user interaction to go to the next phase, 1 - ON, when all condition mets, go automatically to next phase
int heaters;							//number of heaters
int total_power = 0;					//total power of all heaters
int volume = 0;							//objem, ktory sa bude varit (pre priblizny vypocet casu potrebneho na ohrev)
int* section_lines = 0;					//hold lines of CSV file, where the sections begins, redefined later in loop()
int start_of_sections;					//hold the value of the line, where is stored first section
int heating_status = OFF;				//actual status of heating ON or OFF (1 or 0) -1 not defined
int mixing_status = OFF;				//actual status of mixing ON or OFF (1 or 0) -1 not defined
String value;							//string to hold all string read from CSV file (in setup will be memory allocation to not fragment memory
float previous_phase_temperature;		//end temperature of the previous phase
int phase_heating;						//force heating 0 - OFF, 1 - ON, -1 it depends on phase and actual temperature
int phase_mixing;						//force mixing 0 - OFF, 1 - ON -1 it depends on phase and actual temperature

//special character definition
byte HeatingChar[8] = {					//heating character	
	B00100,
	B00100,
	B01110,
	B01110,
	B01110,
	B11111,
	B11111,
	B01110
};

byte MixingChar[8] = {					//mixing character	
	B00000,
	B10000,
	B11001,
	B01011,
	B00100,
	B11010,
	B10011,
	B00001
};


void setup()
{
	Serial.begin(9600);						//define speed for serial communication

	value.reserve(BUFFER_SIZE + 1);			//allocate memory for String value
	buffer[BUFFER_SIZE] = '\0';

	// connect AREF to 3.3V and use that as VCC, less noisy!
	analogReference(EXTERNAL);

	pinMode(PIN_HEATER1, OUTPUT);				//define port for heater #1 as output
	pinMode(PIN_HEATER2, OUTPUT);				//define port for heater #2 as output
	pinMode(PIN_MIXING, OUTPUT);				//define port for Mixer as output

	//Turn off all relay controls
	digitalWrite(PIN_HEATER1, HIGH);
	digitalWrite(PIN_HEATER2, HIGH);
	digitalWrite(PIN_MIXING, HIGH);

	//LCD init
	lcd.init();								// inicializacia LCD
	lcd.backlight();						// zapnutie podsvietenia
	lcd.createChar(0, HeatingChar);			//create char for heating
	lcd.createChar(1, MixingChar);			//create char for mixing

	// Configure all of the SPI select pins as outputs and make SPI
	// devices inactive, otherwise the earlier init routines may fail
	// for devices which have not yet been configured.
	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);

	#if PIN_OTHER_DEVICE_CS > 0
		//  pinMode(ETHERNET_SELECT, OUTPUT);
		//  digitalWrite(ETHERNET_SELECT, HIGH);	// disable Ethernet
	#endif //PIN_OTHER_DEVICE_CS > 0


	// Setup SD card
	if (!sd.begin(PIN_SD_CS, SD_CARD_SPEED))
	{
		lcd.setCursor(0, 0);
		lcd.print(F("SD card begin error"));
		// Cannot do anything else
		while (1) {
			delay(1000);
			Serial.print(F("SD card begin error"));
		}
	}
	//  opening CSV file; O_RDWR | O_CREAT
	if (!csv.open(FILENAME, O_READ)) {
		lcd.setCursor(0, 0);
		lcd.print(F("Failed open file"));
		while (1) {
			delay(1000);
			Serial.print(F("Failed open file"));
		}
	}

	//setup timers
	timer_for_loop.setTimeout(2000);		// expire after 1000ms (1s)
	timer_for_loop.setHertz(1);				// timer is reseting 1 time per second
	timer_for_alarm.setTimeout(30000);		// expire after 30000ms (30s)

	previous_phase_temperature = getTemperature();
}



void loop()
{
	//declaration of variables
	String phase_name;						//string to hold phase name
	int phase_time;							//duration in minutes of actual phase
	unsigned long calc_phase_time = 0;		//calculated phase duration in millis
	//int phase_eta;						//how many minutes is till the end of the phase
	int phase_heating_power;				//heating power in percents, -1 means not defined
	//float phase_temperature_rise = -1;		//reqested temperature rise in minutes, -1 if not
	int phase_mixing_power;					//mixing power in percents, -1 means not defined so it will be as default mixing power
	byte default_heating_power = 100;		//default heating power in percents
	byte default_mixing_power = 100;		//default mixing power in percents
	int phase_mixing_delay;					//mixing delay, so how many minutes to stop the mixing after heating is going off
	float phase_temperature;				//ending temperature of phase
	String phase_message;					//message that is displaing on the end of the phase
	int phase_alarm;						//type of alarm on the end of phase, -1 off, 0 - "stop melody", 1 - "info melody"
	unsigned long phase_start_time;			//start time of phase in millis
	unsigned long phase_actual_time;		//actual time of phase in millis
	int sensorValue;						//value from analog port
	float actual_temperature;				//actual temperature
	int is_heating;							//calculate value based on ending temperature of previous and current phase
											//1 = heating, so ending condition is bigger temperature as previous phase
											//0 = keeping the temperature, so ending condition is the same temperature as previous phase
											//-1 = cooling,  so ending condition is less temperature as previous phase
	
	mixing_status = 0;						//set the mixing_status to zero, later will be set to real status

	//first analyze CSV
	if (actual_phase == 0) {
		//first get number of sections in CSV
		int sections = get_number_of_sectionsCSV();
		//then delete and redefine the arrays to hold the sections positions
		delete[] section_lines;
		section_lines = new int[sections];	// redefine array for holding information about corresponding sections lines in CSV file


		//get sections names and its lines and paste it into array section_lines[]
		//declaration of variable
		int j = 0;
		int line = 0;
		int lines = get_number_of_linesCSV();

		for (int line = 0; line < lines; ++line) {
			line = get_next_selection_lineCSV(line, lines);
			if (line != -1) {
				section_lines[j] = line;
				j++;
			}
			else {
				line = lines;
			}
		}

		start_of_sections = section_lines[0];

		// Fetch the pot volume
		value = get_selection_valueCSV("pot volume", section_lines[0], section_lines[1]);
		volume = atoi(value.c_str());

		// Fetch a number of heaters, it's power and calculate total power
		total_power = heating_total_power();

		//fetch default heating and mixing power
		value = get_selection_valueCSV("default heat power", section_lines[0], section_lines[1]);
		default_heating_power = atoi(value.c_str());
		value = get_selection_valueCSV("default mix power", section_lines[0], section_lines[1]);
		default_mixing_power = atoi(value.c_str());

		
		LCD_print_message(0, 1, 1);				//lcd.setCursor(0, 0); lcd.print(F("Set volume in liters"));	
		lcd.setCursor(0, 1);
		lcd.print(volume);
		LCD_print_message(2, 2, 1);				//lcd.setCursor(0, 2); lcd.print(F("and press Y to start"));
		LCD_print_message(3, 3, 1);				//lcd.setCursor(0, 3); lcd.print(F("[1]< [2]> [3]Y [4]N"));

		while (actual_phase == 0) {
			// update the ResponsiveAnalogRead object every loop
			analog_keypad.update();
			delay(50);
			if (analog_keypad.hasChanged())
			{
				// read the input on analog pin
				sensorValue = analog_keypad.getValue();
				if (key(sensorValue) == 1)	volume = volume - 1;
				if (key(sensorValue) == 2)	volume = volume + 1;
				if (key(sensorValue) == 3) {
					actual_phase = 1;
					latest_phase = 0;
				}
				lcd.setCursor(0, 1);
				lcd.print(volume);
				lcd.print(F("  "));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	/* IF NEW PHASE BEGGINS, GET ALL VALUES OF THE PHASE AND CALCULATE EVERYTHING NEEDED*/
	//////////////////////////////////////////////////////////////////////////////////////

	if (actual_phase - latest_phase == 1)
	{
		lcd.clear();
		//get all needed values for the phase from CSV file
		//get the name of the actual phase
		phase_name = get_phase_name(section_lines[actual_phase]);

		//get exact time for this phase, if 0 there is no exact time
		value = get_selection_valueCSV("phase time", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_time = atoi(value.c_str());

		//get autosteping
		value = get_selection_valueCSV("autosteping", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_autosteping = atoi(value.c_str());

		//get if heating is on or off
		value = get_selection_valueCSV("heating", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_heating = atoi(value.c_str());

		//get heating power
		value = get_selection_valueCSV("heating power", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_heating_power = atoi(value.c_str());
		if (phase_heating_power > 100 || phase_heating_power < 0) phase_heating_power = default_heating_power;

		//get temperature rise
		//value = get_selection_valueCSV("temperature rise", section_lines[actual_phase], section_lines[actual_phase + 1]);
		//phase_temperature_rise = atof(value.c_str());

		//get if mixing is on or off
		value = get_selection_valueCSV("mixing", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_mixing = atoi(value.c_str());

		//get mixing_power
		value = get_selection_valueCSV("mixing power", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_mixing_power = atoi(value.c_str());
		if (phase_mixing_power > 100 || phase_mixing_power < 0) phase_mixing_power = default_mixing_power;

		//get mixing_delay
		value = get_selection_valueCSV("mixing delay", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_mixing_delay = atoi(value.c_str());
		timer_for_mixing_delay.setTimeout(phase_mixing_delay*1000);
		timer_for_mixing_delay.stop();

		//get requested temperature
		previous_phase_temperature = phase_temperature;
		value = get_selection_valueCSV("temperature", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_temperature = atof(value.c_str());

		//get message that is print on the end of the phase
		phase_message = get_selection_valueCSV("message", section_lines[actual_phase], section_lines[actual_phase + 1]);

		//get alarm type
		value = get_selection_valueCSV("alarm", section_lines[actual_phase], section_lines[actual_phase + 1]);
		phase_alarm = atoi(value.c_str());

		//get start of the phase
		phase_start_time = millis();

	}

	//get actual temperature
	actual_temperature = getTemperature();
	//is the phase heating, cooling or keeping the temperature?
	if (phase_temperature > 0)
	{
		if (phase_temperature < actual_temperature) is_heating = COOLING;	//cooling phase
		if (phase_temperature > actual_temperature) is_heating = HEATING;	//heating phase
		if (abs(phase_temperature - actual_temperature) <= TEMP_OFFSET) is_heating = KEEPING;	//keeping temperature
	}

	
	//calculate time for phase from heating power, temperature rise and volume to calculate time
	//calculate temperature rise (delta, gap)

	//if heating is on and time is not important
	if ((phase_time == 0) && (phase_temperature - actual_temperature > 0))
	{
		int tmp_float = phase_heating_power / 100 * total_power;
		calc_phase_time = calc_aprox_time(volume, (phase_temperature - actual_temperature), tmp_float);
	}
	//if heating is on and time is important
	//else if (phase_time > 0 && delta_temperature > 0)
	//{
	//	//TO BE DONE IF YOU WANT TO HEAT THE VOLUME IN EXACT TIME//
	//}
	else if (phase_time != 0)
	{
		calc_phase_time = phase_time * 60000;
		timer_for_phase.setTimeout(calc_phase_time);							//set the timer for phase, if exact time is needed
		timer_for_phase.restart();
	}
	
	
	//show phase name and button options
	lcd.setCursor(0, 0);
	lcd.print(phase_name);
	LCD_print_message(3, 4, 1);				//lcd.setCursor(0, 3); lcd.print(F("[1]Pause     [4]Stop"));

	/////////////////////
	/*ACTUAL PHAZE LOOP*/
	/////////////////////
	while (actual_phase - latest_phase == 1) {

		// every second update the info on display and check the temperature
		if (timer_for_loop.onRestart()) {

			//show heating nad mixing icons
			if (heating_status == ON)
			{
				lcd.setCursor(9, 3);
				lcd.write(0);
			}
			else
			{
				lcd.setCursor(9, 3);
				lcd.print(" ");
			}
			if (mixing_status == ON)
			{
				lcd.setCursor(11, 3);
				lcd.write(1);
			}
			else
			{
				lcd.setCursor(11, 3);
				lcd.print(" ");
			}

			//show the actual time and ETA time of the phase
			lcd.setCursor(0, 1);
			lcd.print(F(" "));
			phase_actual_time = millis();
			TimeDisplay(phase_start_time, phase_actual_time);
			lcd.setCursor(10, 1);
			lcd.print(F(" ["));
			TimeDisplay(0, calc_phase_time);
			lcd.print(F("]"));

			//show actual state of temperatures
			actual_temperature = getTemperature();
			lcd.setCursor(0, 2);
			lcd.print(F("Temp: "));
			lcd.print(actual_temperature);
			lcd.print(F(" ["));
			lcd.print(phase_temperature);
			lcd.print(F("] "));

			//force mixing ON or OFF
			if (mixing_status == 0)
			{
				if (phase_mixing == 1)
				{
					digitalWrite(PIN_MIXING, LOW);
					mixing_status = ON;
				}
				else if (phase_mixing == 0)
				{
					digitalWrite(PIN_MIXING, HIGH);
					mixing_status = OFF;
				}
			}

			//force heating ON or OFF
			if (heating_status != -1) 
			{
				if (phase_heating == 1)
				{
					heating_on_off(ON);
					heating_status = ON;
				}
				else if (phase_heating == 0)
				{
					heating_on_off(OFF);
					heating_status = OFF;
				}
			}
			
			//if this is heating phase, turn the heating on
			if (heating_status != ON && is_heating == HEATING && actual_temperature <= phase_temperature + TEMP_OFFSET) heating_on_off(ON);
			//if (is_heating == HEATING && actual_temperature > phase_temperature + TEMP_OFFSET)
			//{
			//	phase_end(phase_message);
			//	break;
			//}

			//if it is cooling phase, turn the heating off
			if (heating_status != OFF && is_heating == COOLING && actual_temperature >= phase_temperature + TEMP_OFFSET) heating_on_off(OFF);
			//if (is_heating == COOLING && actual_temperature < phase_temperature + TEMP_OFFSET)
			//{
			//	phase_end(phase_message);
			//	break;
			//}

			//if it is keeping temperature phase, keep temperature, till the time is not reached
			if (phase_temperature > 0 && phase_time > 0)
			{
				if (heating_status != ON && actual_temperature <= phase_temperature - TEMP_OFFSET) heating_on_off(ON);
				if (heating_status != OFF && actual_temperature >= phase_temperature + TEMP_OFFSET) heating_on_off(OFF);
			}


			//turn off mixing after mixing delay
			//if (timer_for_mixing_delay.onRestart())
			//{
			//	digitalWrite(PIN_MIXING, HIGH);
			//	mixing_status = OFF;
			//	timer_for_mixing_delay.stop();
			//}

			/////////////////////////////////////////
			/*CHECK IF THIS IS THE END OF THE PHASE*/
			/////////////////////////////////////////
			//is the end of the phase elapsed time?
			if (phase_time > 0 && timer_for_phase.onExpired())		
			{
				phase_end(phase_message);					//call function for phase end
				break;
			}
			//is the end of the phase reached temperature?
			if (phase_time == 0)
			{
				if (is_heating == HEATING && actual_temperature >= phase_temperature + TEMP_OFFSET)			//when heating, the phase end is when the temperature is bigger of the TEMP_OFFSET as actual 
				{
					phase_end(phase_message);											//call function for phase end
					break;
				}
				if (is_heating == COOLING && actual_temperature <= phase_temperature + TEMP_OFFSET)			//when cooling, phase end is when the actual temperature is less then phase temperature + TEMP_OFFSET
				{
					phase_end(phase_message);											//call function for phase end
					break;
				}

			}
		}
	}

	//if program is there, next phase started
	latest_phase = latest_phase + 1;
	Serial.println(actual_phase);
	Serial.println(latest_phase);

	//lcd.setCursor(19, 3);
	//lcd.write(0);

	delay(1000);
}
