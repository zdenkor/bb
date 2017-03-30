//get the string value of the selection name that is on the interval from_line to_line
String get_selection_valueCSV(String selection_name, int from_line, int to_line)
{
	int num_sections = 0;
	for (int line = from_line; line < to_line; ++line) {
		csv.gotoLine(line);
		csv.readField(buffer, BUFFER_SIZE);
		if (selection_name == buffer) {
			csv.nextField();
			csv.readField(buffer, BUFFER_SIZE);
			return buffer;
		}
	}
	return "-1";
}
