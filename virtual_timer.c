// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"
#include "virtual_timer_linked_list.h"

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void) {
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  //printf("Timer Fired! \n");
  NRF_TIMER4->EVENTS_COMPARE[0] = 0;
  NRF_TIMER4->TASKS_CAPTURE[0] = 1;
  uint32_t curr_time =  NRF_TIMER4->CC[0];
  //printf("current time: %u\n", curr_time);
  //list_print();
  node_t* my_node = list_remove_first();
  //list_print();
  //printf("my node value: %u\n", my_node);
  if(my_node != NULL) {
    my_node->callback();
    if(my_node->repeated) {
        node_t* head = NULL;
        head = (node_t *) malloc(sizeof(node_t));
        head->timer_value = curr_time + my_node->frequency;
        head->callback = my_node->callback;
        head->frequency = my_node->frequency;
        head->repeated = true;
        head->id = my_node->id;
        list_insert_sorted(head);
    }
    free(my_node);
  }
  
  node_t * new_first = list_get_first();
  if(new_first != NULL) {
    //printf("here before \n");
    NRF_TIMER4->TASKS_CAPTURE[0] = 1;
    curr_time =  NRF_TIMER4->CC[0];
    if (curr_time >= new_first -> timer_value) {
      //printf("here\n");
      TIMER4_IRQHandler();
      return;
    }
    NRF_TIMER4->CC[0] = new_first -> timer_value;
  }
  //list_print();
  
  // Place your interrupt handler code here
  //printf("Timer Exited! \n");
}



// Read the current value of the timer counter
uint32_t read_timer(void) {

  // Should return the value of the internal counter for TIMER4
  NRF_TIMER4->TASKS_CAPTURE[1] = 1;
  return NRF_TIMER4->CC[1];
  }
// 5) Start the timer
void virtual_timer_init(void) {
  // Place your timer initialization code here
  NRF_TIMER4->BITMODE = 3;
  NRF_TIMER4->PRESCALER = 4;
  NRF_TIMER4->INTENSET = 1 << 16;
  NVIC_EnableIRQ(TIMER4_IRQn);
  NRF_TIMER4->TASKS_CLEAR = 1;
  NRF_TIMER4->TASKS_START = 1;
}


// Start a timer. This function is called for both one-shot and repeated timers
// To start a timer:
// 1) Create a linked list node (This requires `malloc()`. Don't forget to free later)
// 2) Setup the linked list node with the correct information
//      - You will need to modify the `node_t` struct in "virtual_timer_linked_list.h"!
// 3) Place the node in the linked list
// 4) Setup the compare register so that the timer fires at the right time
// 5) Return a timer ID
//
// Your implementation will also have to take special precautions to make sure that
//  - You do not miss any timers
//  - You do not cause consistency issues in the linked list (hint: you may need the `__disable_irq()` and `__enable_irq()` functions).
//
// Follow the lab manual and start with simple cases first, building complexity and
// testing it over time.
static uint32_t timer_start(uint32_t microseconds, virtual_timer_callback_t cb, bool repeated) {

  // Return a unique timer ID. (hint: What is guaranteed unique about the timer you have created?)
  node_t* head = NULL;
  head = (node_t *) malloc(sizeof(node_t));
  head->timer_value = microseconds;
  head->callback = cb;
  head->frequency = microseconds;
  head->repeated = repeated;
  head->id = (uint32_t) head;
  
  list_insert_sorted(head);
  NRF_TIMER4->CC[0] = list_get_first() -> timer_value;
  
  
  
  
  
  return (uint32_t) head;
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, false);
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, true);
}

// Remove a timer by ID.
// Make sure you don't cause linked list consistency issues!
// Do not forget to free removed timers.
void virtual_timer_cancel(uint32_t timer_id) {
  node_t* head = list_get_first();
  list_print();
  printf("first head_val: %u\n", head);
  printf("timer id: %u\n", timer_id);
  while((head != NULL) && ((uint32_t) head != timer_id) && head->id != timer_id){
    printf("curr head_val: %u\n", head);
    head = head->next;
  }
  printf("final head_val: %u\n", head);
  
  if(head != NULL) {
    list_remove(head);
    free(head);
  }
  list_print();

}

