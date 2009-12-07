/*
 *  abstract_fsm.h
 *
 *  Created by David Bergman on 12/5/09.
 *  Copyright 2009 David Bergman. All rights reserved.
 *
 * These definitions abstracts away two FSM libraries - Boost.Statechart and (Boost.)MSM -
 * by using preprocessor constructs (Boost.Preprocessor)
 *
 * This makes it much easier to define machines with states, event and actions, where
 * there is no nested machines nor guards for transitions, i.e., relatively simple
 * state machines.
 */

#include <boost/preprocessor.hpp>


// A state machine specification is pretty simple:
// The name of the machine, a list of events and a list of states along with
// the transitions out of the state - the transition table
// The initial state is assumed to be the first one in the transition table.
// The transition elements are pairs with the source state as the first
// argument and a sequence of outgoing transitions as the second one, where
// those transitions in turn are tuples like:
//
//   (event, destState, action)
//
// TODO: extract events and states from a transition table not already
// partitioned per state

#if USE_SC

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>

#define FSM_DEF(machine, events, trans) \
  BOOST_PP_SEQ_FOR_EACH(EVENT_DEF, machine, events) \
  BOOST_PP_SEQ_FOR_EACH(STATE_FORWARD, machine, STATES(trans)) \
  struct machine : boost::statechart::state_machine<machine, INITIAL_STATE(trans)> { \
    BOOST_PP_SEQ_FOR_EACH_I(ACTION_DEF, machine, TABLE(trans)) \
  }; \
  BOOST_PP_SEQ_FOR_EACH(STATE_DEF, machine, trans)

#elif USE_MSM

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#define FSM_DEF(machine, events, trans)				     \
  BOOST_PP_SEQ_FOR_EACH(EVENT_DEF, machine, events) \
  BOOST_PP_SEQ_FOR_EACH(STATE_DEF, machine, trans) \
  struct machine ## _ : boost::msm::front::state_machine_def<machine ## _> { \
    typedef INITIAL_STATE(trans) initial_state; \
    BOOST_PP_SEQ_FOR_EACH_I(ACTION_DEF, machine ## _, TABLE(trans)) \
    typedef boost::mpl::vector< \
      BOOST_PP_SEQ_FOR_EACH_I(TRANSITION, machine ## _, TABLE(trans)) \
    > transition_table; \
    void initiate() {} \
  }; \
  typedef boost::msm::back::state_machine<machine ## _> machine;

#else
#error You have to specify some underlying FSM implementation, with a USE_<IMPL>, such as USE_SC or USE_MSM
#endif

// A simple forward declaration of state
#define STATE_FORWARD(s, machine, state) struct state;


#if USE_SC


// A state definition from a transition tuple
// TODO: handle cases with zero outgoing transitions
#define STATE_DEF(s, machine, trans) struct STATE(trans) : \
  boost::statechart::simple_state<STATE(trans), machine> { \
    typedef boost::mpl::list< \
      BOOST_PP_SEQ_FOR_EACH_I(TRANSITION, (machine, STATE(trans)), TRANSITIONS(trans)) \
    > reactions; \
};

#define EVENT_DEF(s, machine, evt) struct evt : boost::statechart::event<evt> { static const char* NAME; };\
  const char* evt::NAME = BOOST_PP_STRINGIZE(evt);

#define TRANSITION(s, machineAndState, n, trans) BOOST_PP_COMMA_IF(n)\
  boost::statechart::transition<TRANS_EVENT(trans), TRANS_DEST(trans),\
    BOOST_PP_TUPLE_ELEM(2, 0, machineAndState),\
    &BOOST_PP_TUPLE_ELEM(2, 0, machineAndState)::BOOST_PP_SEQ_CAT((act)(BOOST_PP_TUPLE_ELEM(2, 1, machineAndState))(TRANS_EVENT(trans))(TRANS_DEST(trans)))>

#define FSM_START(machine) machine.initiate()


#elif USE_MSM

#define STATE_DEF(s, machine, trans) struct STATE(trans) : boost::msm::front::state<> {};

#define EVENT_DEF(s, machine, evt) struct evt { static const char* NAME; };\
  const char* evt::NAME = BOOST_PP_STRINGIZE(evt);

#define TRANSITION(s, machine, n, trans) BOOST_PP_COMMA_IF(n)\
  a_row<TABLE_SOURCE(trans), TABLE_EVENT(trans), TABLE_DEST(trans), \
    &machine::BOOST_PP_SEQ_CAT((act)(TABLE_SOURCE(trans))(TABLE_EVENT(trans))(TABLE_DEST(trans)))>

#define FSM_START(machine) 


#endif

// ACTION_DEF acts on table rows, which are quadruplets
#define ACTION_DEF(s, data, n, trans) void BOOST_PP_SEQ_CAT((act)(TABLE_SOURCE(trans))(TABLE_EVENT(trans))(TABLE_DEST(trans)))(const TABLE_EVENT(trans)& evt) TABLE_ACTION(trans)

//
// Auxiliary PP functions to deal with the constituents of our machine
// specifications
//

// Extract initial state out of the transitions
#define INITIAL_STATE(transitions) BOOST_PP_SEQ_HEAD(STATES(transitions))

// Extract all states from the transitions
#define STATES(transitions) BOOST_PP_SEQ_TRANSFORM(PICK_FIRST, 0, transitions)

// Get the state of a state transition structure
#define STATE(trans) BOOST_PP_TUPLE_ELEM(2, 0, trans)

// Get the transitions of a state transition structure
#define TRANSITIONS(trans) BOOST_PP_TUPLE_ELEM(2, 1, trans)

// Translate a state transition which is grouped by state to
// a global one, not grouped, but rather having the source
// state as a constituent of the transition, so
// we have
// (source, event, dest, action)
#define TABLE(trans) BOOST_PP_SEQ_FOR_EACH(STATE_ROWS, 0, trans)

// Extract all events from a table of transitions
#define ALL_EVENTS(trans) BOOST_PP_SEQ_TRANSFORM(PICK_TABLE_EVENT, 0, trans)

// Extract all events from a state-grouped transition table
#define ALL_EVENTS_STATE(trans) ALL_EVENTS(TABLE(trans))

// Collect the rows of a state's outgoing transitions into
// a table-like structure, including the source state in the rows
#define STATE_ROWS(s, data, trans) BOOST_PP_SEQ_TRANSFORM(TABLE_TRANS, STATE(trans), TRANSITIONS(trans))

// Create a table transition row from a state-specific row
#define TABLE_TRANS(s, state, trans) (state, BOOST_PP_TUPLE_ELEM(3, 0, trans), BOOST_PP_TUPLE_ELEM(3, 1, trans), BOOST_PP_TUPLE_ELEM(3, 2, trans))

// Get the destination state of a transition
#define TRANS_DEST(trans) BOOST_PP_TUPLE_ELEM(3, 1, trans)

// Get the event of a transition
#define TRANS_EVENT(trans) BOOST_PP_TUPLE_ELEM(3, 0, trans)

// Get the action (body) of a transition
#define TRANS_ACTION(trans) BOOST_PP_TUPLE_ELEM(3, 2, trans)

// Get the source state of a table transition
#define TABLE_SOURCE(trans) BOOST_PP_TUPLE_ELEM(4, 0, trans)

// Get the destination state of a table transition
#define TABLE_DEST(trans) BOOST_PP_TUPLE_ELEM(4, 2, trans)

// Get the event of a table transition
#define PICK_TABLE_EVENT(s, data, trans) TABLE_EVENT(trans)

// Get the event of a table transition
#define TABLE_EVENT(trans) BOOST_PP_TUPLE_ELEM(4, 1, trans)

// Get the action of a table transition
#define TABLE_ACTION(trans) BOOST_PP_TUPLE_ELEM(4, 3, trans)

#define PICK_FIRST(s, data, pair) BOOST_PP_TUPLE_ELEM(2, 0, pair)


