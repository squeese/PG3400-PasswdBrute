#ifndef BC_TALKIEWALKIE_H
#define BC_TALKIEWALKIE_H

enum {
  TW_CODE_IDLE,
  TW_CODE_EXIT,
  TW_CODE_PING,
  TW_CODE_PONG
};

typedef void*(*tw_state_fn)(int);

int tw_read_code(int);
void tw_send_code(int, int);

#endif
