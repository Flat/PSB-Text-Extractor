#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


//Removes NULL characters and replaces them with NEWLINE
void format_buffer(char *buffer, size_t bufferSize)
{
	for (int i = 0; i < bufferSize; i++)
	{
		if (buffer[i] == '\0') {
			buffer[i] = '\n';
		}
	}
}

unsigned long find_offset(FILE *pFile)
{
	unsigned long offset = 0;
	unsigned char b0, b1, b2, b3;

	//In PSB files 20 bytes in the offset for the text location is found
	if (0 != fseek(pFile, 20, SEEK_SET))
	{
		return -1;
	}

	//Read the offset bytes little-endian style
	fread(&b3, sizeof b3, 1, pFile);
	fread(&b2, sizeof b2, 1, pFile);
	fread(&b1, sizeof b1, 1, pFile);
	fread(&b0, sizeof b0, 1, pFile);
	offset = (((unsigned long) b0) << 24) | (((unsigned long) b1) << 16) | \
	         (((unsigned long) b2) << 8 ) | b3;
	return offset;
}

//Extracts text from input file and writes to output file
int extract_text (char *input_file, char *output_file)
{
	FILE *pFileIn = NULL;
	FILE *pFileOut = NULL;
	printf("Opening file: %s\n", input_file);
	unsigned long offset = 0;


	pFileIn = fopen(input_file, "rb");
	if (pFileIn == NULL)
	{
		printf("Error opening file, aborting.\n");
		return 1;
	}


	printf("Searching for offset...");

	if ((offset = find_offset(pFileIn)) == -1)
	{
		printf("%30s", "Failed.\n");
	}
	else
	{
		printf("%#30lx\n", offset);
	}




	//Set our file cursor to the offset of the text location
	fseek(pFileIn, offset, SEEK_SET);
	pFileOut = fopen(output_file, "w");
	if (pFileOut == NULL)
	{
		printf("Failed to open file for output, aborting.\n");
	}
	char buffer[4096];
	size_t readBytes;

	//UTF8 Header
	char utf8[3];
	utf8[0] = 0xEF;
	utf8[1] = 0xBB;
	utf8[2] = 0xBF;
	utf8[3] = 0x20;


	//Add UTF8 header to our file so programs know how to decode the data
	fwrite(utf8, sizeof(char), sizeof(utf8), pFileOut);

	//Read data from offset to end of file
	while ((readBytes = fread(buffer, sizeof(char), sizeof(buffer), \
	                          pFileIn)) != 0)
	{
		format_buffer(buffer, sizeof(buffer));
		fwrite(buffer, sizeof(char), readBytes, pFileOut);
	}
	fclose(pFileIn);
	fclose(pFileOut);
	printf("Successfully extracted text to %s\n", output_file);
	return 0;
}

int main (int argc, char **argv)
{
	printf("PSB Text Extractor Tool v.00\nAuthor: Flat (http://github.com/Flat)\n\n");
	int option, err = 0;
	int iflag = 0, oflag = 0;
	char *input_file = NULL;
	char *output_file = NULL;
	static char usage[] = "\nusage: %s -i input_file -o output_file\n";

	while ((option = getopt(argc, argv, "i:o:")) != -1)
		switch (option) {
		case 'i':
			input_file = optarg;
			iflag = 1;
			break;
		case 'o':
			oflag = 1;
			output_file = optarg;
			break;
		case '?':
			err = 1;
			break;
		}
	if (iflag == 0) {
		fprintf(stderr, "%s: missing -i option\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		return 1;
	}
	else if (oflag == 0) {
		fprintf(stderr, "%s: missing -o option\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		return 1;
	}
	else if (err) {
		fprintf(stderr, usage, argv[0]);
		return 1;
	}
	extract_text(input_file, output_file);
	return 0;
}