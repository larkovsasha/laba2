#ifndef SHARED_H
#define SHARED_H

#include <string>
#include "string.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cerrno>
#include <sys/file.h>

const std::string SHARED_FILE = "/tmp/message_shared.txt";
const std::string LOCK_FILE = "/tmp/message_shared.lock";

const int BUFFER_SIZE = 16;

const char *PING = "PING";
const char *PONG = "PONG";

const char *CLIENT_MSG = "_CLIENT_MSG";
auto CLIENT_MSG_SIZE = strlen(CLIENT_MSG);
const char *SERVER_MSG = "_SERVER_MSG";
auto SERVER_MSG_SIZE = strlen(SERVER_MSG);
const char *ERROR_MSG = "_ERROR";


bool write_to_shared_file(const std::string &filename, const std::string &data) {
    int lock_fd = open(LOCK_FILE.c_str(), O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) return false;

    if (flock(lock_fd, LOCK_EX) == -1) {
        close(lock_fd);
        return false;
    }

    int file_fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (file_fd == -1) {
        flock(lock_fd, LOCK_UN);
        close(lock_fd);
        return false;
    }

    ssize_t written = write(file_fd, data.c_str(), data.length());
    close(file_fd);

    flock(lock_fd, LOCK_UN);
    close(lock_fd);

    return written > 0;
}

std::string read_from_shared_file(const std::string &filename) {
    int lock_fd = open(LOCK_FILE.c_str(), O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) return "";

    if (flock(lock_fd, LOCK_SH) == -1) {
        close(lock_fd);
        return "";
    }

    int file_fd = open(filename.c_str(), O_RDONLY);
    if (file_fd == -1) {
        flock(lock_fd, LOCK_UN);
        close(lock_fd);
        return "";
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(file_fd, buffer, BUFFER_SIZE - 1);
    close(file_fd);
    flock(lock_fd, LOCK_UN);
    close(lock_fd);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';  // Гарантируем null-termination
        return std::string(buffer, bytes_read);
    }
    return bytes_read > 0 ? std::string(buffer, bytes_read) : "";
}

#endif