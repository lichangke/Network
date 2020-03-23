
#ifndef MESSAGE_STRUCT_H
#define MESSAGE_STRUCT_H



typedef struct {
    u_int32_t type;
    char data[1024];
} messageObject;

#define MSG_PING          1
#define MSG_PONG          2
#define MSG_TYPE1        11
#define MSG_TYPE2        21

#endif