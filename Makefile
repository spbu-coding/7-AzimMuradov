include CONFIG.cfg

CC = gcc
LD = gcc
CFLAGS = -g -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS = -MMD -MP
LDFLAGS =

SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
DEPS = $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.d)
OBJECTS = $(DEPS:%.d=%.o)
EXEC = $(BUILD_DIR)/$(NAME)


TESTS_IN_EXT = in
TESTS_OUT_EXT = out
TESTS_ACTUAL_EXT = actual
TESTS_LOG_EXT = log

TESTS_IN = $(wildcard $(TEST_DIR)/*.$(TESTS_IN_EXT))
TESTS_NAMES = $(TESTS_IN:$(TEST_DIR)/%.$(TESTS_IN_EXT)=%)
TESTS_OUT = $(TESTS_IN:%.$(TESTS_IN_EXT)=%.$(TESTS_OUT_EXT))
TESTS_ACTUAL = $(TESTS_IN:%.$(TESTS_IN_EXT)=%.$(TESTS_ACTUAL_EXT))

TESTS_LOG = $(TESTS_IN:%.$(TESTS_IN_EXT)=%.$(TESTS_LOG_EXT))
TESTS_LOG_OK = "OK"
TESTS_LOG_FAILED = "FAILED"

TESTS_CHECK_CMDS = $(TESTS_NAMES:%=check_%)


.SUFFIXES:

.PHONY: all check $(TESTS_CHECK_CMDS) clean

.SILENT: $(BUILD_DIR) \
         check $(TESTS_CHECK_CMDS) $(TESTS_LOG) $(TESTS_ACTUAL)


all: $(EXEC)


#-------------------------------------------------------- Build --------------------------------------------------------

$(EXEC): $(OBJECTS) $(OBJECTS)
	$(LD) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): $(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

-include $(DEPS)

$(BUILD_DIR): ; mkdir -p $@


#-------------------------------------------------------- Tests --------------------------------------------------------

check: $(TESTS_LOG)
	test_cnt=0; \
	test_failed_cnt=0; \
	\
	for test in $(TESTS_NAMES); do \
		test_cnt=$$((test_cnt + 1)); \
		\
		test_log="$$(cat $(TEST_DIR)/$$test.$(TESTS_LOG_EXT))"; \
		\
		echo "Test \"$$test\" - $$test_log"; \
		if [ $$test_log = $(TESTS_LOG_FAILED) ]; then test_failed_cnt=$$((test_failed_cnt + 1)); fi; \
	done; \
	\
	if [ $$test_failed_cnt = 0 ]; then echo -e "\nEverything is OK"; else echo -e "\nSome tests failed"; fi; \
	echo "$$((test_cnt - test_failed_cnt)) / $$test_cnt"; \
	\
	[ $$test_failed_cnt = 0 ]

$(TESTS_CHECK_CMDS): check_%: $(TEST_DIR)/%.$(TESTS_LOG_EXT)
	test_log="$$(cat $<)"; \
	echo "Test \"$*\" - $$test_log"; \
	[ $$test_log = $(TESTS_LOG_OK) ]

$(TESTS_LOG): $(TEST_DIR)/%.$(TESTS_LOG_EXT): $(TEST_DIR)/%.$(TESTS_ACTUAL_EXT)
	if cmp -s $(TEST_DIR)/$*.$(TESTS_OUT_EXT) $<; then \
		echo $(TESTS_LOG_OK) > $@; \
	else \
		echo $(TESTS_LOG_FAILED) > $@; \
	fi

$(TESTS_ACTUAL): $(TEST_DIR)/%.$(TESTS_ACTUAL_EXT): $(TEST_DIR)/%.$(TESTS_IN_EXT) $(EXEC)
	./$(EXEC) $< >$@


#-------------------------------------------------------- Clean --------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR) $(TESTS_LOG) $(TESTS_ACTUAL)

