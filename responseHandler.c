#include <stdio.h>
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "databaseHandler.c"

#define F_OK 0
#define HEADER_SIZE 1024
#define GET "GET"
#define SC_ACCEPT "200"
#define SC_NOT_FOUND "404"

typedef struct data (*jsonFunction)();

struct HTTPResponse {
    struct data header;
    struct data body;
};

struct mappedRoute {
    char *type;  // e.g. GET, POST
    char *route;
    char *template;  // template filepath
    char *statusCode;  // e.g. 200
};

struct apiRoute {
    char *type;  // e.g. "folders", "files"
    jsonFunction jsonFunc;  // function to be called to get json-like string
    int jsonFuncParam;  // parameter passed to the jsonFunc
};

static struct mappedRoute registeredRoutes[] = {
    {GET, "/", "files/index.html", SC_ACCEPT},

    {GET, "/404", "files/404.html", SC_NOT_FOUND},
};

static struct apiRoute registeredApiRoutes[] = {
    {"folders", getTableJson, DB_FOLDERS_TABLE},
    {"files", getTableJson, DB_FILES_TABLE}
};

static char *allowedExt[] = {"css", "js", "html", "png", "jpg", "jpeg", "json"};
static char *imageExt[] = {"png", "jpg", "jpeg"};


char *getFileExt(char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

int isAllowedExt(char *ext) {
    int allowed = 0;
    int n = sizeof(allowedExt) / sizeof(allowedExt[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(allowedExt[i], ext) == 0) {allowed = 1;}
    }
    return allowed;
}

int isApiRequest(char *requestRoute) {
    char *api = "/api/";
    return strncmp(requestRoute, api, strlen(api)) == 0;
}

struct data getApiBody(char *requestRoute) {
    strtok(requestRoute, "/");  // remove first part of route (/api)
    char *APIrequestType = strtok(NULL, "?");
    char *APIrequestParams = strtok(NULL, "");

    // find matching registered api
    int nRegistered = sizeof(registeredApiRoutes) / sizeof(registeredApiRoutes[0]);
    struct apiRoute apiRoute;
    for (int i = 0; i < nRegistered; i++) {
        struct apiRoute r = registeredApiRoutes[i];
        if (strcmp(r.type, APIrequestType) == 0) {
            apiRoute = r;
            break;
        }
    }

    struct data bodyData = apiRoute.jsonFunc(apiRoute.jsonFuncParam);
    return bodyData;
}

struct mappedRoute getMappedRoute(char *requestType, char *requestRoute) {
    // split request into type and route
    char *routePath = requestRoute + 1;
    char *pathExt = getFileExt(requestRoute);

    int nRegistered = sizeof(registeredRoutes) / sizeof(registeredRoutes[0]);
    struct mappedRoute route = registeredRoutes[nRegistered - 1];  // 404 by default

    // find route
    if (strcmp(pathExt, "") == 0) {
        // find route via registered routes
        for (int i = 0; i < nRegistered; i++) {
            struct mappedRoute r = registeredRoutes[i];
            if (strcmp(requestType, r.type) == 0 && strcmp(requestRoute, r.route) == 0) {
                route = r;
                break;
            }
        }
    } else if (access(routePath, F_OK) == 0 && isAllowedExt(pathExt)) {
        // unregistered but valid
        struct mappedRoute r;
        r.type = GET;
        r.statusCode = SC_ACCEPT;
        r.route = requestRoute;
        r.template = routePath;
        route = r;
    }
    return route;
}

char *getContentType(char *ext) {
    char *type = "text";
    int n = sizeof(imageExt) / sizeof(imageExt[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(imageExt[i], ext) == 0) {
            type = "image";
        }
    }
    if (strcmp(ext, "json") == 0) {
        type = "application";
    }
    return type;
}

struct data getHeader(char *templateExt, char *statusCode, int bodySize) {
    struct data header;
    // char *templateExt = getFileExt(routeMap.template);
    
    header.contents = malloc(HEADER_SIZE);
    memset(header.contents, 0, HEADER_SIZE);
    snprintf(header.contents, HEADER_SIZE,  "HTTP/1.1 %s\r\n"
                                            "Content-Type: %s/%s\r\n"
                                            "Content-Length: %d\r\n"
                                            "Accept-Ranges: bytes\r\n\r\n", statusCode, getContentType(templateExt), templateExt, bodySize);
    
    header.size = strlen(header.contents);
    return header;
}

struct data getBody(struct mappedRoute routeMap) {
    struct data file;
    
    // file size
    FILE *fptr = fopen(routeMap.template, "rb");
    fseek(fptr, 0, SEEK_END);
    file.size = ftell(fptr);
    rewind(fptr);

    // read file
    file.contents = malloc(file.size);
    memset(file.contents, 0, file.size);
    int read = fread(file.contents, 1, file.size, fptr);

    fclose(fptr);
    return file;
}

struct HTTPResponse getResponse(char *request) {
    struct HTTPResponse response;

    char *requestType = strtok(request, " ");
    char *requestRoute = strtok(NULL, " ");

    if (!isApiRequest(requestRoute)) {
        // handle HTTP request
        struct mappedRoute routeMap = getMappedRoute(requestType, requestRoute);
        response.body = getBody(routeMap);
        response.header = getHeader(getFileExt(routeMap.template), routeMap.statusCode, response.body.size);
    } else {
        // handle API request
        response.body = getApiBody(requestRoute);
        response.header = getHeader("json", SC_ACCEPT, response.body.size);
    }
    return response;
}
