PROG		= MyMCU
SOURCES		:= main.cxx

ifndef OPENH323DIR
OPENH323DIR=$(HOME)/h323plus
endif

include $(OPENH323DIR)/openh323u.mak