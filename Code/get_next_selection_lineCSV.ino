//get the next section name on the interval from_line to_line
int get_next_selection_lineCSV(int from_line, int to_line)
{
	int i = 0;
	int len = 0;
	for (int line = from_line; line < to_line; ++line) {
		csv.gotoLine(line);
		csv.readField(buffer, BUFFER_SIZE);
		int len = strlen(buffer);
		for (int num = 0; num < len; ++num) {
			byte val = buffer[num];
			if (isUpperCase(val) || isSpace(val)) {
				i++;
			}
		}
		if (i == len) {
			return line;
		}
		i = 0;
	}
	return -1;
}