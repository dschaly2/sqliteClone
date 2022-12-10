// Author: Drew Schaly
// 12/9/2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Struct for the user input buffer
typedef struct {
	char* buffer;
	size_t buffLength;
	ssize_t inputLength;
} InputBuffer;


// ENUMS for processing input
typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

typedef enum { META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED } MetaCommandResult;

typedef enum { PREPARE_SUCCESS, SYNTAX_ERROR, UNRECOGNIZED_STATEMENT } PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE + 1];
	char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
	StatementType type;
	Row rowToInsert;
} Statement;

#define sizeOfAttribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = sizeOfAttribute(Row, id);
const uint32_t USERNAME_SIZE = sizeOfAttribute(Row, username);
const uint32_t EMAIL_SIZE = sizeOfAttribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

#define TABLE_MAX_PAGES 100
const uint32_t PAGE_SIZE =  4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
	uint32_t numRows;
	void* pages[TABLE_MAX_PAGES];
} Table;

void printRow(Row* row) {
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serializeRow(Row* source, void* destination) {
	memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
	memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
	memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserializeRow(void* source, Row* destination) {
	memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
	memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
	memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* rowSlot(Table* table, uint32_t rowNum) {
	uint32_t pageNum = rowNum / ROWS_PER_PAGE;
	void* page = table->pages[pageNum];
	if (page == NULL) {
		// Allocate memory only when accessing page
		page = table->pages[pageNum] = malloc(PAGE_SIZE);
	}
	uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
	uint32_t byteOffset = rowOffset * ROW_SIZE;
	return page + byteOffset;
}

Table* newTable() {
	Table* table = (Table*)malloc(sizeof(Table));
	table->numRows = 0;
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i] = NULL;
	}
	return table;
}

void freeTable(Table* table) {
	for (int i = 0; table->pages[i]; i++) {
		free(table->pages[i]);
	}
	free(table);
}


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
		int argsAssigned = sscanf(
		inputBuffer->buffer, "insert %d %s %s", &(statement->rowToInsert.id), statement->rowToInsert.username, statement->rowToInsert.email);
		if  (argsAssigned < 3) {
			return SYNTAX_ERROR;
		}
		return PREPARE_SUCCESS;
	}
	if (strcmp(inputBuffer->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return UNRECOGNIZED_STATEMENT;
}

ExecuteResult executeInsert(Statement* statement, Table* table) {
	if (table->numRows >= TABLE_MAX_ROWS) {
		return EXECUTE_TABLE_FULL;
	}

	Row* rowToInsert = &(statement->rowToInsert);

	serializeRow(rowToInsert, rowSlot(table, table->numRows));
	table->numRows += 1;

	return EXECUTE_SUCCESS;
}

ExecuteResult executeSelect(Statement* statement, Table* table) {
	Row row;
	for (uint32_t i = 0; i < table->numRows; i++) {
		deserializeRow(rowSlot(table, i), &row);
		printRow(&row);
	}
	return EXECUTE_SUCCESS;
}

ExecuteResult executeStatement(Statement* statement, Table* table) {
	switch (statement->type) {
		case (STATEMENT_INSERT):
			return executeInsert(statement, table);
		case (STATEMENT_SELECT):
			return executeSelect(statement, table);
	}
}



int main(int argc, char* argv[]) {
	Table* table = newTable();
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
			case (SYNTAX_ERROR):
				printf("Syntax error!\n");
				continue;
			case (UNRECOGNIZED_STATEMENT):
				printf("Unrecognized statement: '%s'.\n", inputBuffer->buffer);
				continue;
		}

		switch (executeStatement(&statement, table)) {
			case (EXECUTE_SUCCESS):
				printf("Executed, yay :)\n");
				break;
			case (EXECUTE_TABLE_FULL):
				printf("Table error: TABLE FULL\n");
				break;
		}
	}
}
