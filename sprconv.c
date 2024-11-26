#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char inputlist[13];
char inputname[13];
char outputname[13];
char inname[13];
char outname[13];
uint8_t* far buffer;
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
};

struct BMP_file* BMP_files;

#define TRUE 1
#define FALSE 0

#ifdef __WATCOMC__
/* __TURBOC__       __WATCOMC__ */   
#define malloc      _fmalloc     
#define calloc      _fcalloc     
#define free        _ffree       
#define realloc     _frealloc    
#define strupr      _strupr   
#define disable     _disable
#define enable      _enable
#define getvect     _dos_getvect
#define setvect     _dos_setvect
#define inportb     inp
#define inport      inpw
#define outportb    outp
#define outport     outpw
#define farmalloc   _fmalloc
#define farfree     _ffree
#define asm         _asm
#define NULL        ((void *)0)

#include <malloc.h>
#include <i86.h>
#endif

void menu();

void quit()
{
    // clear memory and exit the program
    inputname [0] = '\0';
	outputname [0] = '\0';
	inname [0] = '\0';
	outname [0] = '\0';
	farfree(buffer);
    running = 0;
    exit(EXIT_FAILURE); // just to make sure the program definitely quits
}

void load_file(char* inname, uint8_t batch_convert, uint8_t batch_transparency, uint8_t batch_animframes)
{
	FILE* file_ptr;
    uint16_t frame_height;
    int remainder;
    char flag_resp;
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
		scanf(" %c", &flag_resp);
			if (flag_resp == 'y' || flag_resp == 'Y')
				flags = 1;
			else if (flag_resp == 'n' || flag_resp == 'N')
				flags = 0;
		
		printf("How many animation frames does the file contain?\n");
		printf("Write '1' if it's a static sprite.\n");
		scanf("%d", &num_frames);
	}

	else
	{
		flags = batch_transparency;
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
	buffer = (uint8_t far*)malloc(filesize);
	printf("\n");
	
	printf("Starting to read first line...\n");
	
	fseek(file_ptr, 1078, SEEK_SET);
	fread(buffer, 1, filesize, file_ptr);
	fclose(file_ptr);
	
	printf("\n%u bytes read!\n", filesize);
	printf("\nFile read successfully!\n");
	fclose(file_ptr);
	printf("\n");
}

void save_file(char* outname)
{
	int16_t y, x;
	FILE* file_ptr = fopen(outname, "wb+");
    fwrite(&width, 2, 1, file_ptr);
    fseek(file_ptr, 2, SEEK_SET);
    fwrite(&height, 2, 1, file_ptr);
    fseek(file_ptr, 4, SEEK_SET);
    fwrite(&num_frames, 2, 1, file_ptr);
    fseek(file_ptr, 6, SEEK_SET);
    fwrite(&flags, 2, 1, file_ptr);
    fseek(file_ptr, 8, SEEK_SET);
	
	for (y = height-1; y >= 0; y--)
	{
		for (x = 0; x < width; x++)
		{
			fputc(buffer[(y*width)+x], file_ptr);
		}
	}
	fclose(file_ptr);
}

void save_file_RLE(char* outname)
{
	int16_t y, x;
	uint8_t current_index = 0; // current palette index being read
	uint8_t next_index = 0; // next palette index being read
	uint8_t compare_index = 0; // current palette index being compared
	uint8_t index_count = 0; // current count of the same palette index
	FILE* file_ptr = fopen(outname, "wb+");
    fwrite(&width, 2, 1, file_ptr);
    fseek(file_ptr, 2, SEEK_SET);
    fwrite(&height, 2, 1, file_ptr);
    fseek(file_ptr, 4, SEEK_SET);
    fwrite(&num_frames, 2, 1, file_ptr);
    fseek(file_ptr, 6, SEEK_SET);
    fwrite(&flags, 2, 1, file_ptr);
    fseek(file_ptr, 8, SEEK_SET);
	
	for (y = height-1; y >= 0; y--)
	{
		for (x = 0; x < width; x++)
		{
			current_index = buffer[(y * width) + x];
			compare_index = current_index;
			index_count++;
			while (compare_index == (next_index = buffer[(y * width) + (x + 1)]) && x < width - 1 && index_count < 256)
			{
				index_count++;
				x++;
			}
			fwrite(&index_count, 1, 1, file_ptr);
			fwrite(&current_index, 1, 1, file_ptr);
			index_count = 0;
		}
	}
	fclose(file_ptr);
}

void convert_single()
{
	char response;
	char RLE_response;

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
	
	printf("Writing file: %s\n", outname);

	// save file to 7UP format
	printf("Y to convert normally, or R to use RLE.\n");
	scanf(" %c", &RLE_response);
	if (RLE_response == 'y' || RLE_response == 'Y')
		save_file(outname);
	else if (RLE_response == 'r' || RLE_response == 'R')
		save_file_RLE(outname);

	printf("BMP successfully converted into %s!\n", outname);

	// clear memory
	inputname [0] = '\0';
	outputname [0] = '\0';
	inname [0] = '\0';
	outname [0] = '\0';
	farfree(buffer);
    
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
	int num_files = 1;
    int i = 0;
    int transparency;
	int animation_frames;
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
		fscanf(BMP_list_file, "%s %d %d", filename, &transparency, &animation_frames);
		BMP_files[i].filename = malloc(strlen(filename) + 1);
		strcpy(BMP_files[i].filename, filename);
		BMP_files[i].transparency = transparency;
		BMP_files[i].animation_frames = animation_frames;
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
		save_file(conversion_name);

		// clear memory
		farfree(buffer);
	}
	printf("List conversion complete!\n");
	menu();
}

void menu()
{
	char response;

	// ask if user wants to convert a single file or a list of files
	printf("Do you want to convert a single file 'S' or a list 'L'? Enter 'Q' to quit.\n");
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
