#ifndef BC_TALKIEWALKIE_H
#define BC_TALKIEWALKIE_H

enum {
  TW_CODE_OPEN,
  TW_CODE_CLOSE
};

typedef void*(*tw_state_fn)(int);

int tw_read(int);
void tw_send(int, int);

#endif
