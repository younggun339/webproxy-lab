CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

SOURCES = echo.c echo_cli.c echo_serv.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = echo echo_cli echo_serv

# 공통 규칙
%.o: %.c csapp.h
	$(CC) $(CFLAGS) -c $<

# 각각의 타겟 및 의존성 정의
all: $(TARGETS)

# 각 타겟에 대한 빌드 규칙
$(TARGETS): $(OBJECTS) csapp.o
	$(CC) $(CFLAGS) $@.o csapp.o -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS) $(OBJECTS) *~