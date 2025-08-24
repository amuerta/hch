#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define PROF_RESET			"\e[0m"
#define PROF_RED			"\e[38;2;255;0;0m"
#define PROF_GREEN			"\e[38;2;0;255;0m"
#define PROF_BLUE			"\e[38;2;0;0;255m"
#define PROF_YELLOW			"\e[38;2;255;255;0m"

#define PROF_LIGHT_RED		"\e[38;2;255;128;0m"
#define PROF_LIGHT_GREEN	"\e[38;2;128;255;0m"
#define PROF_LIGHT_BLUE 	"\e[38;2;0;128;255m"

#define PROF_MAGNETA		"\e[38;2;255;0;255m"
#define PROF_PINK			"\e[38;2;255;0;127m"
#define PROF_CYAN			"\e[38;2;0;255;255m"


#define	getname(var) #var

void __assert_w_error(bool condition, char* err,char* calle) {
	if (!condition) {
		printf("\n[%sERROR%s] (%s:%s)\t:\n>\t %s \"%s\" %s",
				PROF_RED,
				PROF_RESET,
				__FILE__,
				calle,
				PROF_LIGHT_RED,
				err,
				PROF_RESET
		);
		exit(-1);
	}
}
#define assert_w_error(CON,ERR) __assert_w_error((CON),(ERR),(char*)__func__)

void printl(char* msg) {
	printf("\n[%sLOG%s]:\t \"%s\"",
			PROF_LIGHT_GREEN,
			PROF_RESET,
			msg
	);
}

void printw(char* msg) {
	printf("\n[%sWARN%s]:\t \"%s\"",
			PROF_LIGHT_RED, // orange
			PROF_RESET,
			msg
	);
}


long profile_get_time_us(void) {
	struct timespec time = {0};
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return (time.tv_sec) * 1000000 +
			(time.tv_nsec) / 1000;
} 

long profile_time_us(void) {
	static bool flush 				= false;
	static struct timespec start	= {0};
	struct timespec end 			= {0};
	long result 					= 0;

	if (!flush) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		flush++;
		return 0;
	}
	else {
		flush = false;
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		result = (end.tv_sec - start.tv_sec) * 1000000 +
			(end.tv_nsec - start.tv_nsec) / 1000;
		memset(&start,0,sizeof(struct timespec));
		memset(&end,0,sizeof(struct timespec));

	}
	return result;
}

long profile_time_ms(void) {
	return profile_time_us() / 1000;
}

