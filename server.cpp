#include "shared.h"

class Server {
private:
    bool running;
    enum class State {
        WAIT_REQUEST,
        PROCESS_REQUEST,
        HANDLE_ERROR,
        SEND_RESPONSE
    };
    State current_state;
    std::string received_message;
    const std::string PONG_ = PONG;

public:
    Server() : running(true), current_state(State::WAIT_REQUEST) {
        cleanup_shared_files();
    }

    void run() {
        std::cout << "=== Сервер запущен ===" << std::endl;
        while (running) {
            switch (current_state) {
                case State::WAIT_REQUEST:
                    wait_request_state();
                    break;
                case State::PROCESS_REQUEST:
                    process_request_state();
                    break;
                case State::HANDLE_ERROR:
                    handle_error_state();
                    break;
                case State::SEND_RESPONSE:
                    send_response_state();
                    break;
            }
            usleep(200000); // 02s
        }
    }

    ~Server() {
        cleanup_shared_files();
    }

private:
    void wait_request_state() {
        std::string request = read_from_shared_file(SHARED_FILE);
        if (request.empty()) {
            return;
        }
        if (request.find(CLIENT_MSG) == 0) {
            received_message = request.substr(CLIENT_MSG_SIZE);
            std::cout << "Сервер: получено сообщение от клиента: \"" << received_message << "\"" << std::endl;
            current_state = State::PROCESS_REQUEST;
            return;
        }
        if (request.find(CLIENT_MSG) != 0 && request.find(SERVER_MSG) != 0 && request.find(ERROR_MSG) != 0) {
            current_state = State::HANDLE_ERROR;
        }
    }

    void process_request_state() {
        std::cout << "Сервер: обработка сообщения" << std::endl;
        if (received_message != PING) {
            std::cout << "Сервер: получен не пинг" << std::endl;
            current_state = State::HANDLE_ERROR;
        } else {
            current_state = State::SEND_RESPONSE;
        }
    }

    void send_response_state() {
        std::cout << "Сервер: отправка ответа клиенту" << std::endl;
        if (write_to_shared_file(SHARED_FILE, SERVER_MSG + PONG_)) {
            std::cout << "Сервер: ответ отправлен: \"" << PONG_ << "\"" << std::endl;
            std::cout << std::endl;
            current_state = State::WAIT_REQUEST;
        } else {
            std::cerr << "Сервер: ошибка отправки ответа" << std::endl;
            current_state = State::HANDLE_ERROR;
        }
    }

    void handle_error_state() {
        std::cout << "Сервер: состояние ошибки" << std::endl;
        cleanup_shared_files();
        if (write_to_shared_file(SHARED_FILE, ERROR_MSG)) {
            std::cout << "Сервер: ошибка отправлена." << std::endl;
            std::cout << std::endl;
            current_state = State::WAIT_REQUEST;
        } else {
            exit(1);
        }
        sleep(1);
        current_state = State::WAIT_REQUEST;
    }

    void cleanup_shared_files() {
        std::remove(SHARED_FILE.c_str());
        std::remove(LOCK_FILE.c_str());
        std::cout << "Сервер: очищены shared файлы" << std::endl;
    }
};

int main() {
    Server server;
    server.run();
    return 0;
}