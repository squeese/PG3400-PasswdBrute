#ifndef BC_TALKIEWALKIE_H
#define BC_TALKIEWALKIE_H

extern const char TW_LOG_PREFIX[];
extern const char TW_DEFAULT_UNIX_PATH[];

enum {
  TW_CODE_IDLE,
  TW_CODE_HASH,
  TW_CODE_WORK,
};

typedef void*(*tw_state_fn)(int, void**);

int tw_read_code(int, int*);
void tw_send_code(int, int);

#endif
