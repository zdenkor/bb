/*FUNCTION TO PRINT MESSAGE FROM SD CARD IDENTIFIED BY INDEX MESSAGE ON LINE LINE
line where 0-3
message_index is number index stored as string of the message stored on the CSV file "1","2","3",...
align: 1-left, 0-middle, -1-right
*/

void LCD_print_message(byte line_in_LCD, int line_in_CSV, char align)
{
	//get the message from CSV file
	csv.gotoLine(line_in_CSV);
	csv.readField(buffer, BUFFER_SIZE);
	value = buffer;
	//change message according to align
	int i = 20 - value.length();
	for (int j = 0; j < i; ++j)
	{
		if (align == 1) 
		{
			value.concat(" ");
		}
		else if (align == -1) value = " " + value;
		else
		{
			if ((j % 2) == 0) { value.concat(" "); }
			else { value = " " + value; }
		}
	}
	//print message to line
	lcd.setCursor(0, line_in_LCD);
	lcd.print(value);
}
