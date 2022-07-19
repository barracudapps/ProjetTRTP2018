#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define WIN_FULL -2
#define WIN_IGNORE -1
#define WIN_NOT_IN_SEQUENCE 0

//definition of a window.
typedef struct window{
  uint8_t size : 5; //the current size of the window
  uint8_t last : 5; // the last in-sequence number
	uint32_t nis; // not in sequence : binary representation of the receive window (=timestamp)
  uint8_t crossed : 3; // number of windows filled
} win_t;

// Size of the window
#define WINDOW_SIZE 31

/* Allocates and initialises a struct window.
 * @return: NULL if an error occures
 */
win_t* win_new();

/* Frees a window and all related resources.
 * @win the window to be freed
 */
void win_del(win_t* win);

/* Indicates to the window that a packet has been received.
 * @win the target window
 * @seq the sequence number a the received packet
 * @return WIN_IGNORE if seq is unacceptable  or already in the window,
 * the difference between "last" before and after win_receive if seq is in sequence,
 * and WIN_NOT_IN_SEQUENCE if not.
 */
int win_receive(win_t* win, uint8_t seq);

uint8_t win_get_seqnum(win_t* win);
