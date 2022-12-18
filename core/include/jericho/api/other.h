#ifndef API_OTHER_H_
#define API_OTHER_H_

#include "api/api.h"

API(Python, {})
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (args.find("command") == args.end()) {
        return JsonResponse::error(404, "Command not provided");
    }

    // #define BUF_MAX 1000000

    FILE *fp;
    int status;
    // char path[BUF_MAX];
    std::string result;

    std::string command = args["command"];

    std::string clients = "";
    if (containsKey(args, std::string("clients"))) {
        clients = args["clients"];
    }

    std::string command_path = "python3 ./py/" + command + ".py " + clients;

    fp = popen(command_path.c_str(), "r");
    if (fp == NULL) { return JsonResponse::error(404, "Invalid command provided"); }

    // printf("Size of pipe: %i\n", (int)sz);
    // BLU("Size of fp: %i\n", sizeof(fp));

    int someLen = 0;
    int lineLen = 10000;
    char *line;
    line = (char *)malloc(sizeof(char) * lineLen);
    if (line == NULL) {
        // Something went horribly wrong
        // exit(1);
    }
    while (fgets(line, lineLen, fp)) {
        // Do something to find the size
        BMAG("LINE: %s\n", line);
        result += std::string(line);
        someLen += lineLen;
        if (someLen != lineLen) {
            line = (char *)realloc(line, sizeof(char) * someLen);
        }
        if (line == NULL) {
            // Something went horribly wrong
            // exit(1);
            BRED("LINE IS NULL\n");
        }
    }

    // fseek(fp, 0L, SEEK_END);
    // size_t sz = ftell(fp);
    // rewind(fp);

    // while (fgets(path, BUF_MAX, fp) != NULL) {
    //     printf("%s", path);
    //     result += std::string(path);
    // }
    printf("Size of line is: %i\n", (int)result.size());

    // result = std::string(line);
    BGRE("LINE: %s\n", result.c_str());

    free(line);

    status = pclose(fp);
    if (status == -1) {
        /* Error reported by pclose() */
        return JsonResponse::error(404, "Invalid command provided");
    }

    BGRE("DONE");

    return JsonResponse::success(200, result);

}

API(Spawn, {})
    BBLU("ATTEMPTING TO SPAWN NEW SERVER\n");
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (!subset(std::set<std::string>{"port", "threads", "directory"}, keys(args))) {
        return JsonResponse::error(404, "Invalid arguments provided");
    }

    if (!apiValidate(args["port"], std::regex("^[0-9][0-9]{0,4}[0-9]$"))) {
        return JsonResponse::error(404, "Port is not a number. Should check if in use.");
    }

    if (!apiValidate(args["threads"], std::regex("^[0-9][0-9]{0,3}[0-9]$|^[0-9]$"))) {
        return JsonResponse::error(404, "Thread is not a number");
    }

    if (std::stoi(args["threads"]) > 10) {
        return JsonResponse::error(404, "Threads must be <= 10");
    }

    if (apiValidate(args["directory"], std::regex("\\.\\.|\\s"))) {
        return JsonResponse::error(404, "Directory contains .. or space");
    }

    if (args["directory"].size() > 100) {
        return JsonResponse::error(500, "Directory length greater than 100");
    }

    std::string directory;
    BYEL("PRE-DIRECTORY: %s\n", args["directory"].c_str());
    if (args["directory"].find("%2f") != std::string::npos){
        std::vector<std::string> chunks = tokenize(args["directory"], "%2F");
        for (auto c : chunks) {
            YEL("CHUNK: %s\n", c.c_str());
        }
        directory = join(chunks, "/");
    }

    BBLU("DIRECTORY TO SPAWN IS: %s\n", directory.c_str());

    std::string command;
    std::string port = args["port"];
    std::string threads = args["threads"];
    if (directory != "") {
        command = "./bin/extend -p " + port + " -t " + threads + " -d \"" + directory + "\"";
    } else {
        command = "./bin/extend -p " + port + " -t " + threads;
    }

    int i = system(command.c_str());

    // if (result != "PIPE ERROR") {
        ClusterNode* node = new ClusterNode("127.0.0.1", port, directory, router->cluster()->index());
        router->cluster()->addNode(node);
        router->cluster()->index()->dump();
    // }

    // only allow 5 threads for now until proper error checking and conversion

    return JsonResponse::success(200, "Spawned a new server at host:" + port);
}

#endif