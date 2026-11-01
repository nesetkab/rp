BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CPPFLAGS += $(if $(BREW_PREFIX),-I$(BREW_PREFIX)/include)
LDLIBS += -lncurses
PREFIX ?= /usr/local
BUILD ?= build/release
TARGET ?= rp

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean debug install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $@

debug:
	$(MAKE) BUILD=build/debug TARGET=rp-debug \
		CXXFLAGS="-std=c++17 -Wall -Wextra -g -O0 -fsanitize=address,undefined" \
		LDFLAGS="-fsanitize=address,undefined"

install: rp
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 rp $(DESTDIR)$(PREFIX)/bin/rp

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/rp

clean:
	rm -rf build rp rp-debug

-include $(DEPS)
