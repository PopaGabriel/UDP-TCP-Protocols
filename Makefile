CFLAGS = -Wall -g

# Portul pe care asculta serverul (de completat)
PORT = 12346

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1

ID_CLIENT1 = C1
ID_CLIENT2 = C2

all: server subscriber

# Compileaza server.c
server: server.cpp

# Compileaza client.c
subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber1 run_subscriber2

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber1:
	./subscriber ${ID_CLIENT1} ${IP_SERVER} ${PORT}

# Ruleaza clientul
run_subscriber2:
	./subscriber ${ID_CLIENT2} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
