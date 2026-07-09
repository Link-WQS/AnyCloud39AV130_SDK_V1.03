CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -I.
LIBS = -lm

# cJSON library
CJSON_SRC = cJSON.c
CJSON_OBJ = cJSON.o

# Main tool
TOOL_NAME = dss_config_tool
TOOL_SRC = dss_config_tool.c
TOOL_OBJ = dss_config_tool.o

# Test programs
TEST_PROGRAMS = cjson_test realistic_test json_file_test
TEST_SRCS = cjson_test.c realistic_test.c json_file_test.c
TEST_OBJS = cjson_test.o realistic_test.o json_file_test.o

.PHONY: all clean test

all: $(TOOL_NAME)

$(TOOL_NAME): $(TOOL_OBJ) $(CJSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(TOOL_OBJ): $(TOOL_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CJSON_OBJ): $(CJSON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test programs
cjson_test: cjson_test.o cJSON.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

realistic_test: realistic_test.o cJSON.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

json_file_test: json_file_test.o cJSON.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TOOL_OBJ) $(CJSON_OBJ) $(TOOL_NAME) $(TEST_OBJS) $(TEST_PROGRAMS)

test: cjson_test realistic_test json_file_test
	@echo "Running cjson_test..."
	./cjson_test
	@echo "Running realistic_test..."
	./realistic_test
	@echo "All tests passed!"
