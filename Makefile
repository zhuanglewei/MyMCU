PROG		= MyMCU
SOURCES		:= connection.cxx buffer.cxx main.cxx ep.cxx  channel.cxx 
ifndef OPENH323DIR
OPENH323DIR=$(HOME)/h323plus
endif

include $(OPENH323DIR)/openh323u.mak