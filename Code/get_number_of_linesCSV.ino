//function to count the number of section in CSV file
int get_number_of_linesCSV()
{
	char buffer[BUFFER_SIZE + 1];				//setting the buffer
	buffer[BUFFER_SIZE] = '\0';
	int line = 0;
	csv.gotoBeginOfFile();
	while (csv.nextLine()) {
		line++;
	}
	return line;
	Serial.print(line);
}
