/* TODO: - add getopt.h to add options
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define ARRSZ(a) (sizeof(a)/sizeof(a[0]))

#define TAPE_CHUNK 64
#define OP_ADD     '+'
#define OP_SUB     '-'
#define MV_INC     '>'
#define MV_DEC     '<'
#define IO_INP     ','
#define IO_OUT     '.'
#define LOOP_S     '['
#define LOOP_E     ']'

// Ansii escape codes
#define COL_END     "\x1b[0m"
#define COL_BLK     "\x1b[0;5;0m"
#define COL_RED     "\x1b[0;0;31m"
#define COL_RED_BLK "\x1b[0;5;31m"

int    interpret(char tape[], size_t size);
char * floadTape(const char filename[], size_t * tape_size);
char * readLine(const char prompt[], int size);

int main(int argc, char * argv[])
{
	size_t tape_sz = 0;
	char * tape     = NULL;
	if (argc == 1) {
		tape_sz = 128;
		for (;;) {
			tape = readLine("%~ ", tape_sz);
			sscanf(tape, " %s", tape);
			if (!strncmp(tape, "quit", tape_sz)) {
				free(tape);
				break;
			} else {
				interpret(tape, tape_sz);
			}
			free(tape);
			putchar('\n');
		}
	} else {
		tape = floadTape(argv[1], &tape_sz);
		if (interpret(tape, tape_sz)) {
			free(tape);
			return EXIT_FAILURE;
		}
	}
		
	return EXIT_SUCCESS;
}


/* Load the entire input file into a tape (string) to
 * ease the interpretation
 */
char * floadTape(const char filename[], size_t * tape_size)
{
	FILE * f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "%sError:%s couldn't open the file '%s'.\n",
			COL_RED_BLK, COL_END, filename);	
		return NULL;
	}

	size_t t_size = TAPE_CHUNK;
	char * t =  calloc(t_size, sizeof(char));
	int ch   = getc(f);
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

int interpret(char tape[], size_t size)
{
	if (!tape) {
		fprintf(stderr,
			"%sError:%s couldn't load the tape\n",
			COL_RED_BLK, COL_END);
		return 1;
	}

	char   et[size];
	size_t et_pos  = 0;
	memset(&et, 0x0, size);
	// executable tape
	size_t st_sz   = 64;
	size_t * stack = calloc(st_sz, sizeof(size_t));
	size_t st_pos  = 0;
	
	for (size_t i = 0; i < size; ++i) {
		switch (tape[i]) {
		case (OP_ADD): ++et[et_pos]; break;
		case (OP_SUB): --et[et_pos]; break;
		case (MV_INC):
			if (et_pos != size) {
				++et_pos;
			} else {
				fprintf(stderr,
					"%sError:%s tape overrun in instruction %zu\n",
					COL_RED_BLK, COL_END, i+1);
				free(stack);
				return 1;
			}
			break;			
		case (MV_DEC):
			if (et_pos != 0) {
				--et_pos;
			} else {
				fprintf(stderr,
					"%sError:%s tape underrun in instrunction %zu\n",
					COL_RED_BLK, COL_END, i+1);
				free(stack);
				return 1;
			}
			break;
		case (LOOP_S):
			if (st_pos != st_sz) {
				stack[st_pos++] = i;
			} else {
				st_sz += 64;
				stack = reallocarray(stack, st_sz, sizeof(size_t));
				stack[st_pos++] = i;
			}
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
		default: continue; // brainfuck comment or empty space
		}
	}

	free(stack);
	return 0;
}

char * readLine(const char prompt[], int size)
{
	char * buff = calloc(size, sizeof(char));
	
	printf("%s", prompt);
	fgets(buff, size, stdin);

	return buff;
}
