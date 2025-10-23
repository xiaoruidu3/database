#include <iostream>
#include <cstring>
#include <memory>
#include <cstdint>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;


typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;


typedef enum { META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND } MetaCommandResult;

typedef enum { PREPARE_SUCCESS, PREPARE_SYNTAX_ERROR, PREPARE_STRING_TOO_LONG, PREPARE_NEGATIVE_ID, PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;


#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;


void serialize_row(Row *row, void *destination) {
    std::memcpy(static_cast<char *>(destination) + ID_OFFSET, &(row->id), ID_SIZE);
    std::memcpy(static_cast<char *>(destination) + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    std::memcpy(static_cast<char *>(destination) + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
    std::memcpy(&(destination->id), static_cast<char *>(source) + ID_OFFSET, ID_SIZE);
    std::memcpy(&(destination->username), static_cast<char *>(source) + USERNAME_OFFSET, USERNAME_SIZE);
    std::memcpy(&(destination->email), static_cast<char *>(source) + EMAIL_OFFSET, EMAIL_SIZE);
}


const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;


void *row_slot(Table *table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == nullptr) {
        page = table->pages[page_num] = new char[PAGE_SIZE];
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;

    return static_cast<char *>(page) + byte_offset;
}


InputBuffer *new_input_buffer() {
    auto input_buffer = new InputBuffer();
    input_buffer->buffer = nullptr;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void close_input_buffer(InputBuffer *input_buffer) {
    delete input_buffer->buffer;
    delete input_buffer;
}


void print_prompt() { printf("db > "); }

void read_input(InputBuffer *input_buffer) {
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if (bytes_read <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    // Ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
    if (std::strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_INSERT;

    char* keyword = std::strtok(input_buffer->buffer, " ");
    char* id_string =  strtok(NULL, " ");
    char* username =  strtok(NULL, " ");
    char* email =  strtok(NULL, " ");

    if (id_string == nullptr || username == nullptr || email == nullptr) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = std::atoi(id_string);
    if (id < 0)
        return PREPARE_NEGATIVE_ID;
    if (std::strlen(username) > COLUMN_USERNAME_SIZE)
        return PREPARE_STRING_TOO_LONG;
    if (std::strlen(email) > COLUMN_EMAIL_SIZE)
        return PREPARE_STRING_TOO_LONG;

    statement->row_to_insert.id = id;
    std::strcpy(statement->row_to_insert.username, username);
    std::strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement) {
    if (std::strncmp(input_buffer->buffer, "insert", 6) == 0)
        return prepare_insert(input_buffer, statement);


    if (std::strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}


void print_row(Row *row) {
    std::cout << "("<< row->id << ", " << row->username << ", " << row->email << ")\n";
    // printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

ExecuteResult execute_insert(Statement *statement, Table *table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row *row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table) {
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}


ExecuteResult execute_statement(Statement *statement, Table *table) {
    switch (statement->type) {
        case STATEMENT_INSERT:
            return execute_insert(statement, table);
        case STATEMENT_SELECT:
            return execute_select(statement, table);
    }
}

Table *new_table() {
    auto table = new Table();
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = nullptr;
    }
    return table;
}

Table free_table(Table *table) {
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        free(table->pages[i]);
    }
    free(table);
}

int main(int argc, char *argv[]) {
    Table *table = new_table();
    InputBuffer *input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    std::cout << "unrecognized command " << input_buffer->buffer << "\n";
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_SYNTAX_ERROR:
                std::cout << "Syntax error. Could not parse statement.\n";
                continue;
            case PREPARE_NEGATIVE_ID:
                std::cout << "ID must be positive.\n\n";
                continue;
            case PREPARE_STRING_TOO_LONG:
                std::cout << "String is too long.\n";
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "unrecognized statement " << input_buffer->buffer << "\n";
                continue;
        }
        switch (execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                std::cout << "Executed.\n";
                break;
            case EXECUTE_TABLE_FULL:
                std::cout << "Error: Table full.\n";
                break;
            default:
                break;
        }
    }
}
