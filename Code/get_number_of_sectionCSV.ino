//function to count the number of section in CSV file
int get_number_of_sectionsCSV()
{
	int num_sections = 0;
	int i = 0;
	int len = 0;
	int line = 0;
	csv.gotoBeginOfFile();
	while (csv.nextLine()) {
		if (line == 0) {
			csv.gotoBeginOfFile();
		}
		csv.readField(buffer, BUFFER_SIZE);
		int len = strlen(buffer);
		for (int num = 0; num < len; ++num) {
			byte val = buffer[num];
			if (isUpperCase(val) || isSpace(val)) {
				i++;
			}
		}
		if (i == len) {
			num_sections++;
		}
		i = 0;
		line++;
	}
	return num_sections;
}
