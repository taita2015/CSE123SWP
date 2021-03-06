#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>
#include "common.h"



//add here
#include "crc.h"

//Linked list functions
int ll_get_length(LLnode *);
void ll_append_node(LLnode **, void *);
LLnode * ll_pop_node(LLnode **);
void ll_destroy_node(LLnode *);

//Print functions
void print_cmd(Cmd *);

//Time functions
long timeval_usecdiff(struct timeval *, 
                      struct timeval *);

//TODO: Implement these functions

//DONE! @A@
char * convert_frame_to_char(Frame *);
Frame * convert_char_to_frame(char *);

//新添加函数
char * frame_add_crc_8(char *char_frame);
int frame_corruped(char * char_frame);

Frame * frame_add_dst_src(Frame * frame,char dst,char src);
void frame_get_dst_src(Frame * frame,char *dst,char *src);


Frame * frame_add_seq_num(Frame * frame,char seq);
void frame_get_seq_num(Frame * frame,char *seq);

Frame * frame_add_type(Frame * frame,char type);
void frame_get_type(Frame * frame,char *type);

Frame * frame_add_ack_num(Frame * frame,char ack);
void frame_get_ack_num(Frame * frame,char *ack);

#endif
