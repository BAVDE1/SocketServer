#include <stdio.h>
#include "sqlite3.c"

#define DB_NAME "internal.db"
#define DB_FOLDERS_TABLE 1
#define DB_FILES_TABLE 0


struct data {
    int size;
    char *contents;
};

struct folder {
    int id;
    const unsigned char *name;
    int folder_order;
};

struct file {
    int id;
    const unsigned char *name;
    const unsigned char *path;
    int folder_id;
    const unsigned char *last_updated;
};


sqlite3 *connectToDB() {
    // Dont forget to close the connection
    sqlite3 *db;
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        printf("ERROR: failed to open db (%s)", sqlite3_errmsg(db));
        return 0;
    }
    return db;
}

char *getFolderJsonFromStmt(sqlite3_stmt *stmt) {
    // return json-like string containing elements of the given folder row from db
    char *string = "";

    struct folder fldr;
    fldr.id = sqlite3_column_int(stmt, 0);
    fldr.name = sqlite3_column_text(stmt, 1);
    fldr.folder_order = sqlite3_column_int(stmt, 2);

    sprintf(string, "{id: %d, name: '%s', folder_order: %d},", fldr.id, fldr.name, fldr.folder_order);
    return string;
}

char *getFileJsonFromStmt(sqlite3_stmt *stmt) {
    // return json-like string containing elements of the given file row from db
    char *string = "";

    struct file file;
    file.id = sqlite3_column_int(stmt, 0);
    file.name = sqlite3_column_text(stmt, 1);
    file.path = sqlite3_column_text(stmt, 2);
    file.folder_id = sqlite3_column_int(stmt, 3);
    file.last_updated = sqlite3_column_text(stmt, 4);

    sprintf(string, "{id: %d, name: '%s', path: '%s', folder_id: %d, last_updated: '%s'},", file.id, file.name, file.path, file.folder_id, file.last_updated);
    return string;
}

struct data getTableJson(int DBtable) {
    // return json-like string of rows from given table
    sqlite3 *db = connectToDB();
    sqlite3_stmt *stmt;
    char *query = "SELECT * FROM folders";
    if (DBtable == DB_FILES_TABLE) {
        query = "SELECT * FROM files";
    }
    
    // prep query
    int prep = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    char *objects = malloc(1024);
    memset(objects, 0, 1024);

    // loop through each row returned from query, store json-like string into `objects`
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char *addition;
        switch (DBtable) {
        case DB_FOLDERS_TABLE:
            addition = getFolderJsonFromStmt(stmt);
            break;
        case DB_FILES_TABLE:
            addition = getFileJsonFromStmt(stmt);
            break;
        default:
            break;
        }
        snprintf(objects, strlen(objects) + strlen(addition) + 1, "%s%s", objects, addition);  // +1 for null
    }

    struct data data;
    if (strlen(objects) > 0) {
        objects[strlen(objects)-1] = '\0';  // remove trailing comma
        data.size = strlen(objects) + 3;  // +2 for square brackets, +1 for null
        data.contents = malloc(data.size);
        snprintf(data.contents, data.size, "[%s]", objects);
    } else {
        // no objects, return empty list
        data.size = 3;
        data.contents = "[]";
    }

    int f = sqlite3_finalize(stmt);
    int c = sqlite3_close(db);
    return data;
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
