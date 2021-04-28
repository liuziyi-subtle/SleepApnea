# set compiler defaults for OSX versus *nix
OS := $(shell uname)
ifeq ($(OS), Darwin)
ifndef CC
export CC = $(if $(shell which clang), clang, gcc)
endif
else
# linux defaults
ifndef CC
export CC = gcc
endif
endif

export CFLAGS = -Wall -g3 -pedantic -fPIC
export DFLAGS = -DDEBUG=1
export LDLIBS = -lm

# OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
OBJS = breath_analysis.o arm_spline_interp_init_f32.o arm_spline_interp_f32.o fir_filter.o biquad_filter.o

libsleepbreath.a: $(OBJS)
	ar -r libsleepbreath.a $(OBJS)

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@ $(DFLAGS)
breath_analysis.o:	breath_analysis.c
	$(CC) $(CFLAGS) -c breath_analysis.c -o breath_analysis.o $(DFLAGS) -I/Users/liuziyi/Documents/GitLab/SleepApnea/Debug

arm_spline_interp_init_f32.o:	arm_spline_interp_init_f32.c
	$(CC) $(CFLAGS) -c arm_spline_interp_init_f32.c -o arm_spline_interp_init_f32.o $(DFLAGS) -I/Users/liuziyi/Documents/GitLab/SleepApnea/Debug

arm_spline_interp_f32.o:	arm_spline_interp_f32.c
	$(CC) $(CFLAGS) -c arm_spline_interp_f32.c -o arm_spline_interp_f32.o $(DFLAGS) -I/Users/liuziyi/Documents/GitLab/SleepApnea/Debug

fir_filter.o:	fir_filter.c
	$(CC) $(CFLAGS) -c fir_filter.c -o fir_filter.o $(DFLAGS) -I/Users/liuziyi/Documents/GitLab/SleepApnea/Debug

biquad_filter.o: biquad_filter.c
	$(CC) $(CFLAGS) -c biquad_filter.c -o biquad_filter.o $(DFLAGS) -I/Users/liuziyi/Documents/GitLab/SleepApnea/Debug


clean:
	rm -f $(CURDIR)/*.o $(CURDIR)/*.a