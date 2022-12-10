// Author: Drew Schaly
// 12/9/2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	char* buffer;
	size_t buffLength;
	ssize_t inputLength;
} InputBuffer;

InputBuffer* newInputBuffer() {
	InputBuffer* inputBuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	inputBuffer->buffer = NULL;
	inputBuffer->buffLength = 0;
	inputBuffer->inputLength = 0;

	return inputBuffer;
}

void printPrompt() { printf("(.) > "); }

void readInput(InputBuffer* inputBuffer) {
	ssize_t bytesRead = getline(&(inputBuffer->buffer), &(inputBuffer->buffLength), stdin);

	if (bytesRead <= 0) {
		printf("Failed to read input! :(\n");
		exit(EXIT_FAILURE);
	}

	inputBuffer->inputLength = bytesRead - 1;
	inputBuffer->buffer[bytesRead - 1] = 0;
}

void closeInputBuffer(InputBuffer* inputBuffer) {
	free(inputBuffer->buffer);
	free(inputBuffer);
}


int main(int argc, char* argv[]) {
	InputBuffer* inputBuffer = newInputBuffer();

	while(1){
		printPrompt();

		readInput(inputBuffer);

		if (strcmp(inputBuffer->buffer, "exit") == 0) {
			closeInputBuffer(inputBuffer);
			exit(EXIT_SUCCESS);
		} else {
			printf("IDK that command bro: '%s' \n", inputBuffer->buffer);
		}
	}
}
