#include <stdio.h>
#include <stdlib.h>

int width;
int height;
int max_value;
int input_type;
int output_type;

typedef struct {
	unsigned char r,g,b,a;
} Pixel;

/*
	Used to peek at next char within file. 
	fh: File handle
	c: point to store char
*/
char* peek(FILE* fh, char* c){
	// Pulls first char, stores it, then puts it back.
	*c = fgetc(fh);
	ungetc(*c,fh);
	return c;
}


/*
	Checks if there is comment, if so it skips until a new line. 
	fh: File handle
*/
void skip_comments(FILE* fh){
	char c = 'a';
	// Check next char.
	peek(fh, &c);
	if (c == '#'){
		// Skips until finds new line.
		while (c = fgetc(fh), c != '\n'){}
	}
}

/*
	Reads an int from file.
	fh: File handle
*/
int read_value_from_header(FILE* fh, int i){
	// Checks if next value is comment and skips it.
	skip_comments(fh);
	fscanf(fh,"%i ",&i);
	return i;
}

int read_header(FILE* fh){	
	char c;
	c = fgetc(fh);
	// Checks if magic number begins with P.
	if (c != 'P'){
		fprintf(stderr,"Invalid image file, first magic number: %c\n",c);
		return 1;
	}
	// Check if appropiate file formats
	c = fgetc(fh);
	input_type = atoi(&c);
	if (input_type != 3 && input_type != 6){
		fprintf(stderr,"Invalid PPM file, PPM file type not supported. %c\n",c);
		return 1;
	}
	// Saves input type as number
	
	// Skips down line.
	fgetc(fh);
	// Reads in metadata.
	int i;
	width = read_value_from_header(fh, i);
	height = read_value_from_header(fh, i);
	max_value = read_value_from_header(fh, i);
	return 0;
}
/* 
	Writes header into output file
	fh: output file handle
*/
void write_header(FILE* fh){
	// writes out header to file stream
	fprintf(fh, "P%i\n%i %i\n%i\n", output_type, width, height, max_value);
}

/*
	Reads in a type 6 PPM file to buffer.
	fh: file handle
	buffer: buffer to store file data. 
*/
void read_type_6(FILE* fh, Pixel* buffer){
	// Creates a sub buffer to read in all file data
	int *sub_buffer = malloc(sizeof(int) * width * height);
	// Reading in file data
	fread(sub_buffer,sizeof(int),width * height, fh);
	int row, col, bin;
	Pixel pix;
	for (row = 0; row < height; row += 1){
		for (col = 0; col < width; col += 1){
			// Changes data from binary to Pixel structure format
			bin = sub_buffer[row * width + col];
			pix = buffer[row * width + col];
			pix.a = bin >> 24;
			pix.b = bin >> 16 & 0xFF;
			pix.g = bin >> 8 & 0xFF;
			pix.r = bin & 0xFF;
			
			buffer[row * width + col] = pix;
		}
	}
	// Frees buffer
	printf("\n");
	free(sub_buffer);
}


/*
	
*/
void write_type_6(FILE* fh, Pixel* buffer){
	// Creates a sub_buffer to write all data to before file
	int *sub_buffer = malloc(sizeof(int) * width * height);
	// Reading in file data
	int row, col;
	Pixel pix;
	for (row = 0; row < height; row += 1){
		for (col = 0; col < width; col += 1){
			pix = buffer[row * width + col];
			// Or bytes together to transform Pixel structure to binary
			sub_buffer[row * width + col] = pix.a << 24 | pix.b << 16 | pix.g << 8 | pix.r;
		}
	}
	// Writes sub_buffer to file
	fwrite(sub_buffer,sizeof(int),width * height, fh);
	
	// Frees buffer
	free(sub_buffer);
}

int main(int argc, char* argv[]){
	// Checks if proper amount of arguments
	if (argc !=  4){
		fprintf(stderr, "Proper Usage: ppmrw output_type input_file output_file\n");
		return 1;
	}
	
	if (output_type = atoi(argv[1]), output_type != 3 && output_type != 6){
		fprintf(stderr, "Invalid file type for output file.\n");
		return 1;
	}
	
	// Opens file and checks if any access problems.
	FILE* in = fopen(argv[2], "rb");
	if (in == NULL){
		fprintf(stderr, "File access error\n");
		return 1;
	}
	FILE* out = fopen(argv[3], "wb");
	if (out == NULL){
		fprintf(stderr, "File write error\n");
	}
	
	// Checks if had error when reading header.
	int i;
	if (i = read_header(in), i != 0){
		return i;
	}
	
	// Create buffer to store photo
	Pixel* buffer = malloc(sizeof(Pixel) * width * height);
	
	// Reads in photo
	read_type_6(in, buffer);
	
	// Writes photo to file
	write_header(out);
	write_type_6(out, buffer);
	
	// Clean up
	free(buffer);
	return 0;
}
