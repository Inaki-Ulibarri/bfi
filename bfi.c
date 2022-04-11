/**
 * TODO: - add an interactive prompt
 *       - make the nested looping more robust
 *         against overflows
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define ARRSZ(a) (sizeof(a)/sizeof(a[0]))

#define TAPE_CHUNK 64
#define OP_ADD    '+'
#define OP_SUB    '-'
#define MV_INC    '>'
#define MV_DEC    '<'
#define IO_INP    ','
#define IO_OUT    '.'
#define LOOP_S    '['
#define LOOP_E    ']'

#define COL_END     "\x1b[0m"
#define COL_BLK     "\x1b[0;5;0m"
#define COL_RED     "\x1b[0;0;31m"
#define COL_RED_BLK "\x1b[0;5;31m"

int interpret(char * tape, size_t size);
char * loadTape(const char filename[], size_t * tape_size);

int main(int argc, char * argv[])
{
	if (argc != 2) {
		fprintf(stderr,
			"%s: <file> \n"
			"Input File required \n",
			argv[0]);
		return EXIT_FAILURE;
	}

	size_t tape_sz = 0;
	char * tape = loadTape(argv[1], &tape_sz);
	if (!tape) {
		fprintf(stderr, "%sError:%s loading the file to the tape.\n",
			COL_RED_BLK, COL_END);
	}
	
	interpret(tape, tape_sz);
	
	free(tape);
	return EXIT_SUCCESS;
}

/**
 * Load the entire input file into a tape to
 * ease the interpretation
 */
char * loadTape(const char filename[], size_t * tape_size)
{
	FILE * f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Error opening the file '%s'.\n",
			filename);
		return 0x0;
	}

	size_t t_size = TAPE_CHUNK;
	char * t =  calloc(t_size, sizeof(char));
	int ch = getc(f);
	size_t i = 0;

	for (; ch != EOF; ch = getc(f), ++i) {
		if (t_size < i + 1) {
			t_size += TAPE_CHUNK;
			t = reallocarray(t, sizeof(char), t_size);
		}
		t[i] = ch;
	}
	
	fclose(f);
	*tape_size = t_size;
	return t;
}

/**
 * Actually interpret and execute the tape
 */
int interpret(char * tape, size_t size)
{
	char   et[size];
	memset(&et, 0x0, size);
	size_t et_pos     = 0;
	// executable tape
	size_t stack[256] = {0};
	size_t st_pos     = 0;
		
	for (size_t i = 0; i < size; ++i) {
		switch (tape[i]) {
		case (OP_ADD): ++et[et_pos]; break;
		case (OP_SUB): --et[et_pos]; break;
		case (MV_INC):
			if (et_pos != size) {
				++et_pos;
			} else {
				fprintf(stderr, "%sError:%s tape overrun in instruction %zu\n",
					COL_RED_BLK, COL_END, i+1);
				return 1;
			}
			break;			
		case (MV_DEC):
			if (et_pos != 0) {
				--et_pos;
			} else {
				fprintf(stderr, "%sError:%s tape underrun in instrunction %zu\n",
					COL_RED_BLK, COL_END, i+1);
				return 1;
			}
			break;
		case (LOOP_S):
			stack[st_pos++] = i;
			break;
		case (LOOP_E):
			if (et[et_pos]) {
				i = stack[st_pos-1];
			} else {
				--st_pos;
			}
			break;
		case (IO_INP): et[et_pos] = getchar(); break;
		case (IO_OUT): putchar(et[et_pos]); break;			
		default: break; // brainfuck comment or empty space
		}
	}
	return 0;
}
