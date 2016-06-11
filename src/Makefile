# makefile
#
# used to compile *.c (which located in . and ./irc ./ui) to object file
# output ot <PROJECT DIR>/build

INC = -Iinc 

SRCS = $(wildcard *.c */*.c)
OBJS = $(patsubst %.c, ../build/%.o, $(notdir $(SRCS)))

default:
	./cmd_list.sh --gen
	make $(OBJS)
	./cmd_list.sh --restore

../build/%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c $(GTK3FLAGS) $(PY3FLAGS) $^ -o $@ $(GTK3LIBS) $(PY3LIBS) $(CURLLIBS)

../build/%.o: */%.c
	$(CC) $(INC) $(CFLAGS) -c $(GTK3FLAGS) $(PY3FLAGS) $^ -o $@ $(GTK3LIBS) $(PY3LIBS) $(CURLLIBS)
