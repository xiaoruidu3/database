// #include<iostream>
// #include<fcntl.h>
// #include<unistd.h>
// #include<cinttypes>
// #include<cstring>
//
// #define COLUMN_USERNAME_SIZE 32
// #define COLUMN_USER_EMAIL_SIZE 256
// #define PAGE_SIZE 4096
// #define MAX_PAGE_SIZE 100
//
// typedef struct {
//     int id;
//     char usr_name[COLUMN_USERNAME_SIZE];
//     char user_email[COLUMN_USER_EMAIL_SIZE];
// } Row;
//
// typedef struct {
//     int fd;
//     void* page[MAX_PAGE_SIZE];
// } Pager;
//
// typedef struct {
//     uint32_t row_num;
//     Pager* pager;
// } Table;
//
//
// /***
//  * define row info
//  ***/
// #define size_of_attributes(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
//
// const uint32_t ID_SIZE = size_of_attributes(Row, id);
// const uint32_t USERNAME_SIZE = size_of_attributes(Row, usr_name);
// const uint32_t USER_EMAIL_SIZE = size_of_attributes(Row, user_email);
//
// const uint32_t ID_OFFSET = 0;
// const uint32_t USER_NAME_OFFSET = ID_OFFSET + ID_SIZE;
// const uint32_t USER_EMAIL_OFFSET = USER_NAME_OFFSET + USERNAME_SIZE;
// const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + USER_EMAIL_SIZE;
//
// // insert by append
// void insert_row(Table* table, Row* row) {
//     if (table->byte_size <= 0) {
//         std::cout << "create new space for data" << std::endl;
//         table->data = new char[PAGE_SIZE]; //
//         table->byte_size = 0;
//     }
//
//     std::memcpy(table->data + table->byte_size + ID_OFFSET, &row->id_, sizeof(row->id_));
//     std::memcpy(table->data + table->byte_size + USER_NAME_OFFSET, &row->usr_name_, sizeof(row->usr_name_));
//     std::memcpy(table->data + table->byte_size + USER_EMAIL_OFFSET, &row->user_email_, sizeof(row->user_email_));
//     table->byte_size += ROW_SIZE;
//
// }
//
// void select_all(Table* table) {
//     uint32_t row_count = table->byte_size / ROW_SIZE;
//     Row row;
//     for (uint32_t i = 0; i < row_count; ++i) {
//         std::memcpy(&row.id_, table->data + (i * ROW_SIZE), sizeof(row.id_));
//         std::memcpy(&row.usr_name_, table->data + (i * ROW_SIZE) + USER_NAME_OFFSET, sizeof(row.usr_name_));
//         std::memcpy(&row.user_email_, table->data + (i * ROW_SIZE) + USER_EMAIL_OFFSET, sizeof(row.user_email_));
//         std::cout << row.id_ << " " << row.usr_name_ << "  " << row.user_email_ << std::endl;
//     }
//
// }
//
//
//
//
//
// // goal1: read all db (POC)
// // goal2: read part of db (db could larger than ram, or it's efficient to read whole db file)
// Table* open_db(const char* filename) {
//     // exit db?
//
//     // create db, return
//
//     // otherwise return the last b
//     Table* table = new Table();
//     int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
//     if (fd == -1) {
//         std::cout << "Error opening file: " << errno << std::endl;
//         exit(EXIT_FAILURE);
//     }
//
//     table->fd =fd;
//     off_t byte_size = lseek(fd, 0, SEEK_END);
//     lseek(fd, 0, SEEK_SET); // set it to the start of the file
//     table->data = new char[PAGE_SIZE];
//     size_t read_byte = read(fd, table->data, byte_size);
//     table->byte_size = byte_size;
//     if (read_byte == -1) {
//         std::cout << "Error reading file: " << errno << std::endl;
//         exit(EXIT_FAILURE);
//     }
//
//     return table;
// }
//
// void close_db(Table* table) {
//     write(table->fd, table->data, table->byte_size);
//     delete table->data;
//     delete table;
// }
//
// int main() {
//     char* filename;
//     Table* db = open_db("mydb.db");
//     Row row{0, "xiaorui", "xiaoruipython@gmail.com"};
//
//     int i = 0;
//
//     while (i < 2) {
//         row.id_ += 1;
//         insert_row(db, &row);
//
//         ++i;
//     }
//
//     select_all(db);
//     close_db(db);
//
//     return 0;
//
//
// }

#include "Pager.h"


int main() {
    Table table("mydb.db");
    int i = 0;
    Row row{0, "xiaoruidu", "xiaorui@gmail.com"};
    while (i < 10) {
        ++i;
        row.id = i;
        table.insert_row(row);
    }
    table.select_all();
    return 0;
}