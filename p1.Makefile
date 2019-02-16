#Single Author info:
#rtnaik Rohit Naik
#Group info:
#nphabia Niklesh Phabiani
#rtnaik	Rohit Naik
#anjain2 Akshay Narendra Jain

# Makefile for building program that parallelizes derivative calculation
CC = gcc
CFLAGS = -lm -O3

my_rtt: my_rtt.c
	$(CC) $(CFLAGS) -o my_rtt my_rtt.c
