#pragma once

#include <cinttypes>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <cstring>


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_USER_EMAIL_SIZE 256
#define PAGE_SIZE 4096
#define MAX_PAGE_NUM 100

#define size_of_attributes(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct {
    int id;
    char usr_name[COLUMN_USERNAME_SIZE];
    char user_email[COLUMN_USER_EMAIL_SIZE];
} Row;

const uint32_t ID_SIZE = size_of_attributes(Row, id);
const uint32_t USERNAME_SIZE = size_of_attributes(Row, usr_name);
const uint32_t USER_EMAIL_SIZE = size_of_attributes(Row, user_email);

const uint32_t ID_OFFSET = 0;
const uint32_t USER_NAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t USER_EMAIL_OFFSET = USER_NAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + USER_EMAIL_SIZE;

const uint32_t ROW_NUM_PER_PAGE = PAGE_SIZE / ROW_SIZE;


/***
 * 1- poc: in memory insert and select (care about serilize and deserialize, also tech of size_of_attributes())
 *
 *
 ***/

class Pager {
public:
    Pager() {
        row_num_ = 0;
        for (auto &p: page)
            p = nullptr;
    }

    ~Pager() {
        for (auto &p: page)
            delete p;
    }

    char *get_page(std::uint32_t pageno) {
        if (pageno >= MAX_PAGE_NUM) {
            std::cout << "Pager::get_page(): pageno > MAX_PAGE_NUM" << std::endl;
            exit(EXIT_FAILURE);
        }

        // allocate a new page
        if (page[pageno] == nullptr)
            page[pageno] = new char[PAGE_SIZE];

        return page[pageno];
    }

    void insert_serilize(char* des, const Row& source) {
        // copy data to page
        std::memcpy(des + ID_OFFSET,  &source.id, ID_SIZE);
        std::memcpy(des +  + USER_NAME_OFFSET, &source.usr_name, USERNAME_SIZE);
        std::memcpy(des +  + USER_EMAIL_OFFSET, &source.user_email, USER_EMAIL_SIZE);
    }

    void deserilize(Row& row, char* source) {
        std::memcpy(&row.id, source +ID_OFFSET, ID_SIZE);
        std::memcpy(&row.usr_name, source + USER_NAME_OFFSET, USERNAME_SIZE);
        std::memcpy(&row.user_email, source + USER_EMAIL_OFFSET, USER_EMAIL_SIZE);
    }


    std::uint32_t row_num_;
    char *page[MAX_PAGE_NUM];
};


class Table {
public:
    Table(const char *filename) {
        pager_ = new Pager();
    }

    ~Table() {
        delete pager_;
    }

    void insert_row(const Row &row) {
        // step1: get page*
        std::uint32_t row_num = pager_->row_num_;
        std::uint32_t pageno = row_num / ROW_NUM_PER_PAGE;

        char *page = pager_->get_page(pageno);

        // step2: get byte-offset inside the page
        std::uint32_t row_offset = row_num % ROW_NUM_PER_PAGE * ROW_SIZE;

        // step3: serilize the data and store
        pager_->insert_serilize(page + row_offset, row);
        pager_->row_num_ += 1;
    }


    void select_all() {
        uint32_t row_id = 0;
        Row row;
        while (row_id < pager_->row_num_) {
            std::uint32_t pageno = row_id / ROW_NUM_PER_PAGE;
            char *page = pager_->get_page(pageno);
            std::uint32_t row_offset = row_id % ROW_NUM_PER_PAGE * ROW_SIZE;

            pager_->deserilize(row, page + row_offset);

            std::cout << pageno << " " << row.id << " " << row.usr_name << " " << row.user_email << std::endl;
            ++row_id;
        }
    }

    Pager *pager_;
};
