#include <stdio.h>
#include "sqlite3.c"

#define DB_NAME "internal.db"


sqlite3* connectToDB() {
    sqlite3* db;
    int open = 0;
    open = sqlite3_open(DB_NAME, &db);
    
    if (open != SQLITE_OK) {
        printf("ERROR: failed to open db - %s", sqlite3_errmsg(db));
        return 0;
    }
    printf("Successfully connected to database\n");
    return db;
}