#include "util.h"




char * convert_frame_to_char(Frame * frame)
{
    //TODO: You should implement this as necessary
    char * char_buffer = (char *) malloc(MAX_FRAME_SIZE);

    //清空内容
    memset(char_buffer,
           0,
           MAX_FRAME_SIZE);

    //拷贝头部
    memcpy(char_buffer,
           frame->header,
           FRAME_HEADER_SIZE);

    //拷贝数据部分
    memcpy(char_buffer+FRAME_HEADER_SIZE,
           frame->data,
           FRAME_PAYLOAD_SIZE);

    memcpy(char_buffer+FRAME_HEADER_SIZE+FRAME_PAYLOAD_SIZE,
           frame->crc,
           FRAME_CRC_SIZE);

//    char_buffer[MAX_FRAME_SIZE-1] = frame->crc;

    return char_buffer;
}


Frame * convert_char_to_frame(char * char_buf)
{
    //TODO: You should implement this as necessary
    Frame * frame = (Frame *) malloc(sizeof(Frame));

//    memset(frame->data,
//           0,
//           sizeof(char)*sizeof(frame->data));
    memcpy(frame->header,
           char_buf,
           sizeof(char)*sizeof(frame->header));

    memcpy(frame->data, 
           char_buf + FRAME_HEADER_SIZE,
           sizeof(char)*sizeof(frame->data));

//    frame->crc = char_buf[MAX_FRAME_SIZE-1];
    memcpy(frame->crc,
           char_buf + FRAME_HEADER_SIZE+FRAME_PAYLOAD_SIZE,
           sizeof(char)*sizeof(frame->crc));

    return frame;
}


int frame_corruped(char * char_frame){
    return is_corrupted(char_frame,MAX_FRAME_SIZE);
}


char * frame_add_crc_8(char *char_frame){
    char_frame[MAX_FRAME_SIZE-1] = 0x00;
    char crc = crc8(char_frame,MAX_FRAME_SIZE);
    char_frame[MAX_FRAME_SIZE-1] = crc;
    return char_frame;
}


Frame * frame_add_dst_src(Frame * frame,char dst,char src){
    frame->header[0] = dst;
    frame->header[1] = src;
    return frame;
}


void frame_get_dst_src(Frame * frame,char *dst,char *src){
    *dst = frame->header[0];
    *src = frame->header[1];
}



Frame * frame_add_type(Frame * frame,char type){
    frame->header[2] = type;
    return frame;
}

void frame_get_type(Frame * frame,char *type){
    *type = frame->header[2];
}

Frame * frame_add_seq_num(Frame * frame,char seq){
    frame->header[3] = seq;
    return frame;
}

void frame_get_seq_num(Frame * frame,char *seq){
    *seq = frame->header[3];
}

Frame * frame_add_ack_num(Frame * frame,char ack){
    frame->header[4] = ack;
    return frame;
}

void frame_get_ack_num(Frame * frame,char *ack){
    *ack = frame->header[4];
}

//Linked list functions
int ll_get_length(LLnode * head)
{
    LLnode * tmp;
    int count = 1;
    if (head == NULL)
        return 0;
    else
    {
        tmp = head->next;
        while (tmp != head)
        {
            count++;
            tmp = tmp->next;
        }
        return count;
    }
}

void ll_append_node(LLnode ** head_ptr, 
                    void * value)
{
    LLnode * prev_last_node;
    LLnode * new_node;
    LLnode * head;

    if (head_ptr == NULL)
    {
        return;
    }
    
    //Init the value pntr
    head = (*head_ptr);
    new_node = (LLnode *) malloc(sizeof(LLnode));
    new_node->value = value;

    //The list is empty, no node is currently present
    if (head == NULL)
    {
        (*head_ptr) = new_node;
        new_node->prev = new_node;
        new_node->next = new_node;
    }
    else
    {
        //Node exists by itself
        prev_last_node = head->prev;
        head->prev = new_node;
        prev_last_node->next = new_node;
        new_node->next = head;
        new_node->prev = prev_last_node;
    }
}


LLnode * ll_pop_node(LLnode ** head_ptr)
{
    LLnode * last_node;
    LLnode * new_head;
    LLnode * prev_head;

    prev_head = (*head_ptr);
    if (prev_head == NULL)
    {
        return NULL;
    }
    last_node = prev_head->prev;
    new_head = prev_head->next;

    //We are about to set the head ptr to nothing because there is only one thing in list
    if (last_node == prev_head)
    {
        (*head_ptr) = NULL;
        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
    else
    {
        (*head_ptr) = new_head;
        last_node->next = new_head;
        new_head->prev = last_node;

        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
}

void ll_destroy_node(LLnode * node)
{
    if (node->type == llt_string)
    {
        free((char *) node->value);
    }
    free(node);
}

//Compute the difference in usec for two timeval objects
long timeval_usecdiff(struct timeval *start_time, 
                      struct timeval *finish_time)
{
  long usec;
  usec=(finish_time->tv_sec - start_time->tv_sec)*1000000;
  usec+=(finish_time->tv_usec- start_time->tv_usec);
  return usec;
}


//Print out messages entered by the user
void print_cmd(Cmd * cmd)
{
    fprintf(stderr, "src=%d, dst=%d, message=%s\n", 
           cmd->src_id,
           cmd->dst_id,
           cmd->message);
}

