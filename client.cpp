#include "shared.h"

class Client {
private:
    bool running;
    enum class State {
        GET_USER_INPUT,
        SEND_MESSAGE,
        WAIT_RESPONSE,
        READ_RESPONSE,
        HANDLE_ERROR
    };
    State current_state;
    std::string user_message;
//    const std::string PING_ = PING;
    int response_timeout_counter = 0;
    const int MAX_RESPONSE_WAIT = 10;
public:
    Client() : running(true), current_state(State::GET_USER_INPUT) {}

    void run() {
        std::cout << "=== Client started ===" << std::endl;
        std::cout << "Enter 'exit' to exit" << std::endl;
        std::cout << std::endl;

        while (running) {
            switch (current_state) {
                case State::GET_USER_INPUT:
                    get_user_input_state();
                    break;
                case State::SEND_MESSAGE:
                    send_message_state();
                    break;
                case State::WAIT_RESPONSE:
                    wait_response_state();
                    break;
                case State::READ_RESPONSE:
                    read_response_state();
                    break;
                case State::HANDLE_ERROR:
                    handle_error_state();
                    break;
            }
            usleep(800000); // 04s
        }

        std::cout << "Client stopped" << std::endl;
    }

private:
    bool is_valid_message(const std::string &message) {
        const std::string allowed_specials = " .,!?:;- _@#$%&*()[]{}<>/\\|=+~`^'\"";

        for (char c: message) {
            if (c >= 'a' && c <= 'z') continue;
            if (c >= 'A' && c <= 'Z') continue;
            if (c >= '0' && c <= '9') continue;
            if (allowed_specials.find(c) != std::string::npos) continue;

            return false;
        }
        return message == PING;
    }

    void get_user_input_state() {
//        std::cout << "\nВведите сообщение для сервера: ";

        if (!std::getline(std::cin, user_message) || user_message == "exit") {
            running = false;
            return;
        }
        if (user_message.size() >= BUFFER_SIZE || !is_valid_message(user_message)) {
            current_state = State::HANDLE_ERROR;
            return;
        }
        if (!user_message.empty()) {
            current_state = State::SEND_MESSAGE;
        }
//        current_state = State::SEND_MESSAGE;
    }

    void send_message_state() {
        std::cout << "Client: message sending: \"" << user_message << "\"" << std::endl;

        if (write_to_shared_file(SHARED_FILE, CLIENT_MSG + user_message)) {
            std::cout << "Client: message have sent" << std::endl;
            current_state = State::WAIT_RESPONSE;
        } else {
            std::cerr << "Client: error message sending" << std::endl;
            current_state = State::HANDLE_ERROR;
        }
    }

    void wait_response_state() {
        std::cout << "Client: waiting for response" << std::endl;
        current_state = State::READ_RESPONSE;
    }

    void read_response_state() {
        std::string response = read_from_shared_file(SHARED_FILE);

        if (response == ERROR_MSG) {
            std::cerr << "Client: got error from server: " << response << std::endl;
            current_state = State::GET_USER_INPUT;
            return;
        }

        if (!response.empty() && response.find(SERVER_MSG) == 0) {
            std::cout << "Client: got message from server: \"" << response.substr(SERVER_MSG_SIZE) << "\""
                      << std::endl;
            current_state = State::GET_USER_INPUT;
            response_timeout_counter = 0;
        } else {
            ++response_timeout_counter;
            std::cout << "Client: response from server is not ready yet" << std::endl;
            if (response_timeout_counter >= MAX_RESPONSE_WAIT) {
                std::cerr << "Client: timeout waiting" << std::endl;
                current_state = State::HANDLE_ERROR;
            }
        }
    }

    void handle_error_state() {
        std::cout << "Client: error state" << std::endl;
        sleep(1);
        current_state = State::GET_USER_INPUT;
    }
};

int main() {
    Client client;
    client.run();
    return 0;
}