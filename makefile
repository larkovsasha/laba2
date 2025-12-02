CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
TARGET_CLIENT = cliegit add README.mdnt
TARGET_SERVER = server
SOURCES_CLIENT = client.cpp
SOURCES_SERVER = server.cpp

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(SOURCES_CLIENT)
	$(CXX) $(CXXFLAGS) -o $(TARGET_CLIENT) $(SOURCES_CLIENT)

$(TARGET_SERVER): $(SOURCES_SERVER)
	$(CXX) $(CXXFLAGS) -o $(TARGET_SERVER) $(SOURCES_SERVER)

clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER)
	rm -f /tmp/ping_request_fifo /tmp/pong_response_fifo

.PHONY: all clean