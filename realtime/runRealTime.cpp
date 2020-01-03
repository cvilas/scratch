#include <iostream>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <sched.h>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc<3) {
    cerr << "usage: rtprio PRIORITY EXECUTABLE [OPTIONS]" << endl;
    cerr << "Executes executable EXECUTABLE with options OPTIONS" << endl
  << "under the round robin real time scheduler with priority PRIORITY." 
  << endl;
    exit(EXIT_FAILURE);
  }
  sched_param sp;
  sp.sched_priority=atoi(argv[1]);
  clog << "rtprio: priority set to " << sp.sched_priority << endl;
  if (sched_setscheduler(0,SCHED_RR,&sp)==-1) {
    perror("rtprio: sched_setscheduler SCHED_RR");
    exit(EXIT_FAILURE);
  }
  if (setuid(getuid())==-1) {
    perror("rtprio: setuid");
    exit(EXIT_FAILURE);
  }
  if (execv(argv[2],argv+2)==-1) {
    perror("rtprio: execv");
    exit(EXIT_FAILURE);
  } return EXIT_SUCCESS;
}
