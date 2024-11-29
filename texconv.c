#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Def_bit.h"

char inputlist[13];
char inputname[13];
char outputname[13];
char inname[13];
char outname[13];
uint8_t* buffer;
uint16_t filesize = 0;
uint16_t width;
uint16_t height;
uint16_t num_frames;
uint16_t flags;
int checksum1;
int checksum2;
int running = 1;

struct BMP_file {
    char* filename;
    uint8_t transparency;
	uint8_t animation_frames;
	uint8_t RLE;
	uint8_t RLE_rows;
};

struct BMP_file* BMP_files;

#define TRUE 1
#define FALSE 0

void menu();

void quit()
{
    // clear memory and exit the program
    inputname [0] = '\0';
	outputname [0] = '\0';
	inname [0] = '\0';
	outname [0] = '\0';
	free(buffer);
    running = 0;
    exit(EXIT_FAILURE); // just to make sure the program definitely quits
}

void load_file(char* inname, uint8_t batch_convert, uint8_t batch_transparency, uint8_t batch_animframes)
{
	FILE* file_ptr;
	char c;
	uint16_t i = 0;
	int j = 0;
	int lines = 1;
    
    uint16_t frame_height;
    int remainder;
    char transparency_resp;
    char frame_resp;

	printf("Loading file %s...\n", inname);

	file_ptr = fopen(inname, "rb");
	
	// check that the file actually exists
	if (file_ptr == NULL)
	{
		printf("Unable to open file: %s\n", inname);
		printf("Please check the file actually exists\n");
		exit(EXIT_FAILURE);
		// exit will terminate the program here, nothing further will happen
	}
	
	fseek(file_ptr, 0, SEEK_SET);
	fread(&checksum1, 2, 1, file_ptr); //check for BMP file header
	fseek(file_ptr, 28, SEEK_SET);
	fread(&checksum2, 1, 1, file_ptr); // check bit depth
	
	// if file header or bit depth is wrong, then exit
	if (checksum1 != 19778 || checksum2 != 8)
	{
		printf("Fatal error: %s is not a valid 8-bit bitmap!\n", inname);
		printf("This program will now exit.\n");
		exit(EXIT_FAILURE);
		// exit will terminate the program here, nothing further will happen
	}
	
	fseek(file_ptr, 18, SEEK_SET);
	fread(&width, 2, 1, file_ptr);
	printf("Checking file width... %dpx\n", width);
	fseek(file_ptr, 22, SEEK_SET);
	fread(&height, 2, 1, file_ptr);
	printf("Checking file height... %dpx\n", height);
	printf("Checking file dimensions complete!\n");
    
    if (batch_convert == FALSE)
	{
		printf("Does the file contain transparency Y/N?\n");
		scanf(" %c", &transparency_resp);
		if (transparency_resp == 'y' || transparency_resp == 'Y')
			flags |= (FLAGS_TRANSPARENCY);
		
		printf("How many animation frames does the file contain?\n");
		printf("Write '1' if it's a static sprite.\n");
		scanf("%d", &num_frames);
	}

	else
	{
		if (batch_transparency == TRUE)
			flags |= (FLAGS_TRANSPARENCY);
		num_frames = batch_animframes;
	}
    
    // check if it's an animation
    if (num_frames > 1)
    {
        printf("What is the height of one animation frame?\n");
        scanf("%d", &frame_height);
        
        // check that there are no "extra" pixels
        if (height / num_frames != frame_height)
        {
            remainder = height - num_frames * frame_height;
            printf("Warning! The file has %d unused pixels.\n", remainder);
            printf("Errors may occur. Continue Y/N?\n");
            scanf(" %c", &frame_resp);
            if (frame_resp == 'y' || frame_resp == 'Y')
                running = 1;
            else if (frame_resp == 'n' || frame_resp == 'N')
                quit();
        }
    }
	
	filesize = width * height;
	printf("Total file size: %u bytes\n", filesize);
	printf("\n");
	
	printf("Allocating memory...");
	buffer = malloc(filesize);
	printf("\n");
	
	printf("Starting to read first line...\n");
	fseek(file_ptr, -width, SEEK_END);
	c = fgetc(file_ptr);
	
	while (i < filesize)
	{
		buffer[j] = c;
		i++;
		if (i % width == 0)
		{
			lines++;
			fseek(file_ptr, (-width*lines), SEEK_END);
		}
		c = fgetc(file_ptr);
		j++;
	}
	
	printf("\nFile read successfully!\n");
	fclose(file_ptr);
	printf("\n");
}

void save_file(char* outname, uint8_t* source_data, uint16_t data_size)
{
	FILE* file_ptr;
	file_ptr = fopen(outname, "wb+");
    fwrite(&width, 2, 1, file_ptr);
    fseek(file_ptr, 2, SEEK_SET);
    fwrite(&height, 2, 1, file_ptr);
    fseek(file_ptr, 4, SEEK_SET);
    fwrite(&num_frames, 2, 1, file_ptr);
    fseek(file_ptr, 6, SEEK_SET);
    fwrite(&flags, 2, 1, file_ptr);
	fseek(file_ptr, 8, SEEK_SET);
	fwrite(source_data, 1, data_size, file_ptr);
	fclose(file_ptr);
}

void save_file_RLE(char* outname, uint8_t RLE_rows)
{
	uint16_t i = 0;
	uint8_t current_index = 0; // current palette index being read
	uint8_t compare_index = 0; // current palette index being compared
	uint16_t index_count = 0; // current count of the same palette index
	uint16_t index_count_highest = 0; // highest amount of the same palette index;
	uint16_t index_count_max = UINT16_MAX; // maximum count of the same palette index
	uint8_t index_count_bytes; // amount of bytes to write as index count
	
	FILE* file_ptr;

	/* if we want to retain row data (original image width), set "rowed" bit" */
	if (RLE_rows == TRUE)
		flags |= (FLAGS_ROWED_RLE);

	/* first check if we need single- or double-byte RLE by going through the pixel data and 
	checking what's the highest amount of repetitions in a row */
	while (i < filesize)
	{
		current_index = buffer[i];
		compare_index = current_index;
		index_count++;

		while (compare_index == buffer[i + 1] && index_count <= index_count_max)
		{
			if (compare_index != buffer[i + 1])
				break;
			index_count++;
			i++;
		}
		if (index_count > index_count_highest)
			index_count_highest = index_count;
		index_count = 0;
		i++;
	}

	// set relevant flag if we need two bytes per repetition index
	if (index_count_highest > 255)
		flags |= (FLAGS_LARGE_RLE);

	// reset indexes
	i = 0;
	index_count = 0;

	// set index limits
	if ((flags & FLAGS_LARGE_RLE))
	{
		index_count_max = UINT16_MAX;
		index_count_bytes = 2;
	}
	else
	{
		index_count_max = 255;
		index_count_bytes = 1;
	}

	// open file for writing
	file_ptr = fopen(outname, "wb+");
	fwrite(&width, 2, 1, file_ptr);
    fseek(file_ptr, 2, SEEK_SET);
    fwrite(&height, 2, 1, file_ptr);
    fseek(file_ptr, 4, SEEK_SET);
    fwrite(&num_frames, 2, 1, file_ptr);
    fseek(file_ptr, 6, SEEK_SET);
    fwrite(&flags, 2, 1, file_ptr);
    fseek(file_ptr, 8, SEEK_SET);
	
	// go through buffer again, now writing the pixel data to file

	if (RLE_rows == TRUE)
	{
		int x, y;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				current_index = buffer[(y * width) + x];
				compare_index = current_index;
				index_count++;
				while (compare_index == buffer[(y * width) + (x + 1)] && x < width - 1 && index_count < index_count_max)
				{
					index_count++;
					x++;
				}
				fwrite(&index_count, index_count_bytes, 1, file_ptr);
				fwrite(&current_index, 1, 1, file_ptr);
				index_count = 0;
			}
		}
	}

	else
	{
		while (i < filesize)
		{
			current_index = buffer[i];
			compare_index = current_index;
			index_count++;

			while (compare_index == buffer[i + 1] && index_count <= index_count_max)
			{
				if (compare_index != buffer[i + 1])
					break;
				index_count++;
				i++;
			}

			fwrite(&index_count, index_count_bytes, 1, file_ptr);
			fwrite(&current_index, 1, 1, file_ptr);
			index_count = 0;
			i++;
		}
	}
	fclose(file_ptr);
}

void convert_single()
{
	char response;
	char RLE_response;
	uint8_t RLE_rows;

	// take in the file input name
	printf("Enter file to convert (without file extension):\n");
	scanf("%s", inputname);
	sprintf(inname, "%s.bmp", inputname);
	
	// load file into buffer
	load_file(inname, FALSE, 0, 0);
    
	// take in the file output name
	printf("Enter output name (without file extension)\n");
	printf("or write 'same' to save with the BMP name:\n");
	scanf("%s", outputname);
	
	if (strcmp ("same", outputname) == 0)
		sprintf(outname, "%s.7UP", inputname);
	
	else
		sprintf(outname, "%s.7UP", outputname);
	
	// save file to 7UP format
	printf("N to convert normally, R to use rowed RLE, U to use unrowed RLE.\n");
	scanf(" %c", &RLE_response);
	if (RLE_response == 'n' || RLE_response == 'N')
		save_file(outname, buffer, filesize);
	else if (RLE_response == 'r' || RLE_response == 'R')
		RLE_rows = TRUE;
	else if (RLE_response == 'u' || RLE_response == 'U')
		RLE_rows = FALSE;

	save_file_RLE(outname, RLE_rows);

	printf("Writing file: %s\n", outname);

	printf("BMP successfully converted into %s!\n", outname);

	// clear memory
	inputname [0] = '\0';
	outputname [0] = '\0';
	inname [0] = '\0';
	outname [0] = '\0';
	free(buffer);
    
	// ask if user wants to convert another file, or quit
	printf("Y to convert another file, or N to quit.\n");
	scanf(" %c", &response);
	if (response == 'y' || response == 'Y')
		running = 1;
	else if (response == 'n' || response == 'N')
		quit();
}

void convert_list()
{
	FILE* BMP_list_file;
	int num_files = 1; // for whatever reason the Win32 version crashes if this is 0 even though all of the code is the same as DOS
    int i = 0;
    int transparency, animation_frames, RLE;
	int RLE_rows = FALSE;
    char filename[13] = {'\0'};

	fflush(stdin);

	// take in the file list name
	printf("Enter file list to convert (including file extension):\n");
	scanf("%s", inputlist);

    BMP_list_file = fopen(inputlist, "r");

    if (BMP_list_file == NULL)
    {
        fclose(BMP_list_file);
		printf("Invalid bitmap list file.\n");
		printf("This program will now exit.\n");
		exit(EXIT_FAILURE);
		// exit will terminate the program here, nothing further will happen
    }

	BMP_files = malloc(sizeof(struct BMP_file));

	do
	{
		fscanf(BMP_list_file, "%s %d %d %d %d", filename, &transparency, &animation_frames, &RLE, &RLE_rows);
		BMP_files[i].filename = malloc(strlen(filename) + 1);
		strcpy(BMP_files[i].filename, filename);
		BMP_files[i].transparency = transparency;
		BMP_files[i].animation_frames = animation_frames;
		BMP_files[i].RLE = RLE;
		BMP_files[i].RLE_rows = RLE_rows;
		i++;
		num_files++;
		BMP_files = realloc(BMP_files, num_files * sizeof(struct BMP_file));
	} while (!feof (BMP_list_file));
    
    fclose(BMP_list_file);

	// start converting
	for (i = 0; i < num_files - 1; i++)
	{
		char conversion_name[15] = {'\0'};
		// construct file extension
		strncpy(conversion_name, BMP_files[i].filename, (strlen(BMP_files[i].filename) - 4)); // remove .BMP file extension
		strcat(conversion_name, ".7UP"); // add sprite file extension
		load_file(BMP_files[i].filename, TRUE, BMP_files[i].transparency, BMP_files[i].animation_frames);
		printf("Saving file as: %s\n", conversion_name);
		if (BMP_files[i].RLE == TRUE)
		{
			printf("RLE compression enabled.\n");
			save_file_RLE(conversion_name, RLE_rows);
		}
		else
		{
			save_file(conversion_name, buffer, filesize);
		}

		// clear memory
		free(buffer);
		printf("File %d of %d successfully converted.\n\n", i + 1, num_files - 1);
	}
	printf("List conversion complete!\n");
	menu();
}

void menu()
{
	char response;

	// ask if user wants to convert a single file or a list of files
	printf("Do you want to convert a single file 'S' or a list 'L'?\nEnter 'Q' to quit.\n");
	scanf(" %c", &response);
	if (response == 's' || response == 'S')
		convert_single();
	else if (response == 'l' || response == 'L')
		convert_list();
	else if (response == 'q' || response == 'Q')
		quit();
}

int main()
{
	while (running == 1)
		menu();
	return 0;
}