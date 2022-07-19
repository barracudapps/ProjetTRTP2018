#include "window.h"

win_t* win_new(){
  win_t* win =  (win_t*)malloc(sizeof(win_t));
  win->last = 31;
  win->size = WINDOW_SIZE;
  win->nis = 0;
  win->crossed = 7;
  return win;
}

void win_del(win_t* win){
  free(win);
}

int win_receive(win_t* win, uint8_t seq){
  uint32_t bin = 0b1<<(seq%32);
  uint8_t lastseq = (win->last + 32*win->crossed);
  //fprintf(stderr, "Crossed: %d\tLast: %d\n", win->crossed,win->last);
  if(win->nis & bin)
    return WIN_IGNORE;
  if(win->crossed == 7){
    if(seq <= lastseq && seq > win->last){
      //fprintf(stderr, "Crossed: %d\n", win->crossed);
      return WIN_IGNORE;
    }
  }
  else if(seq <= lastseq || seq > (lastseq + 31)){
//    fprintf(stderr, "NIS: %d\n", win->nis);
    //fprintf(stderr, "Not-crossed: %d\n", win->crossed);
    return WIN_IGNORE;
  }
  win->size--;
  win->nis = (win->nis)|bin;
  if(seq%32 == (win->last+1)%32){
    uint8_t last = win->last;
    while(win->nis & bin){
      win->nis -= bin;
      if(win->last == 31){
        win->last = 0;
        win->crossed ++;
      }else{
        win->last++;
      }
      if(bin == (uint32_t)1<<31)
        bin = 0b1;
      else
        bin <<=1;
      win->size++;
    }
//    fprintf(stderr, "NIS: %d\n", win->nis);
    return ((win->last)-last+32)%32;
  }
//  fprintf(stderr, "NIS: %d\n", win->nis);
  return WIN_NOT_IN_SEQUENCE;
}


uint8_t win_get_seqnum(win_t* win){
  return 32*win->crossed + win->last;
}
