// Author: Drew Schaly
// 12/9/2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Struct for the user input buffer
typedef struct {
	char* buffer;
	size_t buffLength;
	ssize_t inputLength;
} InputBuffer;


// ENUMS for processing input

typedef enum { META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED } MetaCommandResult;

typedef enum { PREPARE_SUCCESS, UNRECOGNIZED_STATEMENT } PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
	StatementType type;
} Statement;

// Function to create new input buffer
InputBuffer* newInputBuffer() {
	InputBuffer* inputBuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	inputBuffer->buffer = NULL;
	inputBuffer->buffLength = 0;
	inputBuffer->inputLength = 0;

	return inputBuffer;
}

// Prints shell prompt
void printPrompt() { printf("(.) > "); }

// Function to read input and save values to InputBuffer struct
void readInput(InputBuffer* inputBuffer) {
	ssize_t bytesRead = getline(&(inputBuffer->buffer), &(inputBuffer->buffLength), stdin);

	if (bytesRead <= 0) {
		printf("Failed to read input! :(\n");
		exit(EXIT_FAILURE);
	}

	// Ignore trailing newline
	inputBuffer->inputLength = bytesRead - 1;
	inputBuffer->buffer[bytesRead - 1] = 0;
}

// Freeing memory for buffer struct and input buffer
void closeInputBuffer(InputBuffer* inputBuffer) {
	free(inputBuffer->buffer);
	free(inputBuffer);
}

// Function to process meta commands
MetaCommandResult doMetaCommand(InputBuffer* inputBuffer) {
	if (strcmp(inputBuffer->buffer, ".exit") == 0) {
		closeInputBuffer(inputBuffer);
		exit(EXIT_SUCCESS);
	} else {
		return META_COMMAND_UNRECOGNIZED;
	}
}

// Function to identify statements
PrepareResult prepareStatement(InputBuffer* inputBuffer, Statement* statement) {
	// Use strncmp because input will be followed by data
	if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
		statement->type = STATEMENT_INSERT;
		return PREPARE_SUCCESS;
	}
	if (strcmp(inputBuffer->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return UNRECOGNIZED_STATEMENT;
}

// Function to execute statements
void executeStatement(Statement* statement) {
	switch (statement->type) {
		case (STATEMENT_INSERT):
			printf("=== INSERT ===\n");
			break;
		case (STATEMENT_SELECT):
			printf("--- SELECT ---\n");
			break;
	}
}



int main(int argc, char* argv[]) {
	InputBuffer* inputBuffer = newInputBuffer();

	while(1){
		printPrompt();

		readInput(inputBuffer);
				
		if (inputBuffer->buffer[0] == '.') {
			switch (doMetaCommand(inputBuffer)) {
				case (META_COMMAND_SUCCESS):
					continue;
				case (META_COMMAND_UNRECOGNIZED):
					printf("Unrecognized command: '%s'\n", inputBuffer->buffer);
					continue;
			}
		}
		
		Statement statement;
		switch (prepareStatement(inputBuffer, &statement)) {
			case (PREPARE_SUCCESS):
				break;
			case (UNRECOGNIZED_STATEMENT):
				printf("Unrecognized statement: '%s'.\n", inputBuffer->buffer);
				continue;
		}

		executeStatement(&statement);
		printf("Executed.\n");

	}
}
