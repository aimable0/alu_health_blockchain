CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -Wno-deprecated-declarations
LDFLAGS = -lssl -lcrypto
TARGET = alu_health_chain
SRCS = main.c crypto.c merkle.c mempool.c ledger.c insurance.c mining.c persistence.c
OBJS = $(SRCS:.c=.o)
DEPS = blockchain.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "\n  [+] Build successful: ./$(TARGET)\n"

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "  Cleaned build artifacts."

reset: clean
	rm -f health_chain.dat private_key.pem public_key.pem
	@echo "  Removed keys and saved chain."

re: clean all