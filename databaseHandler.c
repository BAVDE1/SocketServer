#include <stdio.h>
#include "sqlite3.c"

#define DB_NAME "internal.db"
#define DB_FOLDERS_TABLE "folders"


sqlite3 *connectToDB() {
    // Dont forget to close the connection
    sqlite3 *db;
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        printf("ERROR: failed to open db (%s)", sqlite3_errmsg(db));
        return 0;
    }
    return db;
}

int initialiseDB() {
    char *foldersTableQuery = "CREATE TABLE IF NOT EXISTS 'folders' ("
                                "'id'	INTEGER NOT NULL UNIQUE,"
                                "'name'	TEXT,"
                                "'folder_order'	INTEGER,"
                                "PRIMARY KEY('id' AUTOINCREMENT));";
    executeRawSql(foldersTableQuery);

    char *filesTableQuery = "CREATE TABLE IF NOT EXISTS 'files' ("
                            "'id'	INTEGER NOT NULL UNIQUE,"
                            "'name'	TEXT,"
                            "'path'	TEXT,"
                            "'folder_id'	INTEGER,"
                            "'last_updated'	TEXT,"
                            "PRIMARY KEY('id' AUTOINCREMENT));";
    executeRawSql(filesTableQuery);

    // char *params[2] = {"YARharhar", "1"};
    // executeSqlParams("INSERT INTO folders(name, folder_order) VALUES(?, ?)", params, 2);
    printf("Initialised database!\n");
    return 1;
}

int executeRawSql(char *query) {
    // Execute raw SQL on the database
    int returnVal = 1;
    sqlite3 *db = connectToDB();

    int e = sqlite3_exec(db, query, NULL, NULL, NULL);
    if (e != SQLITE_OK) {
        printf("ERROR: Failed to execute query (%d)", e);
        returnVal = 0;
    }
    sqlite3_close(db);
    return returnVal;
}

int executeSqlParams(char *query, char **params, int nParams) {
    // Put `nParams` number of `params` array into `query`, and execute
    int returnVal = 1;
    sqlite3 *db = connectToDB();
    sqlite3_stmt *stmt;

    // prep query
    int prep = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (prep == SQLITE_ERROR) {
        printf("ERROR: '?' Parameter cannot go there\n");
        returnVal = 0;
    }
    
    // bind nParams number of params
    for (int i = 0; i < nParams; i++) {
        if (sqlite3_bind_text(stmt, i + 1, params[i], -1, SQLITE_STATIC)) {
            printf("ERROR: binding param number %d to param %s\n", i, params[i]);
            returnVal = 0;
        }
    }
    
    // execute query
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: Failed to execute statement (step)\n");
        returnVal = 0;
    }
    
    if (sqlite3_finalize(stmt)) {
        printf("ERROR: Failed to finalize execution\n");
        returnVal = 0;
    }
    sqlite3_close(db);
    return returnVal;
}
