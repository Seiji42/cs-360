# Makefile for message client and server

CXX=			g++ $(CCFLAGS)

# MESSAGE-SERVER=		message-server.o inet-server.o server.o
MESSAGE-CLIENT=		message-client.o inet-client.o client.o
OBJS =			$(MESSAGE-CLIENT)
# $(MESSAGE-SERVER) 

LIBS=
MESSAGE
CCFLAGS= -g

all:	message-client
	# message-server 

# message-server:$(MESSAGE-SERVER)
# 	$(CXX) -o server $(MESSAGE-SERVER) $(LIBS)

message-client:$(MESSAGE-CLIENT)
	$(CXX) -o client $(MESSAGE-CLIENT) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) server client


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
