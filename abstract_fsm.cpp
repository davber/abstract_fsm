#include <iostream>
#include "abstract_fsm.h"

FSM_DEF(Machine, (Stop)(Play), // machine and its events
	// The transitions (including the states)
	((Stopped,
	  ((Play, Playing,  { std::cout << "starting to play with event " << evt.NAME << std::endl; }))
	  ((Stop, Stopped,  { std::cout << "stopping" << std::endl; }))))
	((Playing,
	  ((Stop, Stopped,  { std::cout << "staying in Stopped" << std::endl; })))
	 ))

int main(int argc, const char* argv[])
{
  Machine mach;
  mach.initiate();
  mach.process_event(Stop());
  mach.process_event(Play());
  mach.process_event(Stop());
}
