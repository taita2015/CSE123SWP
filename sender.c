#include "sender.h"

void init_sender(Sender * sender, int id)
{
    //TODO: You should fill in this function as necessary
    sender->send_id = id;
    sender->input_cmdlist_head = NULL;
    sender->input_framelist_head = NULL;

    sender->seq_num = 20;
    sender->LAR = 0;
    sender->LAF = 0;

    for(int i =0;i<SWS;i++){
        sender->sendQ[i].frame = NULL;
    }

}

struct timeval * sender_get_next_expiring_timeval(Sender * sender)
{
    //TODO: You should fill in this function so that it returns the next timeout that should occur
    return NULL;
}



//NFE - Next frame expected
//LFR - Sequence number of largest consecutive frame received
//LAF - Sequence number of largest acceptable frame
//RWS - Max receive window size
//LFR = NFE - 1
//LAF = NFE + RWS - 1

int sendQ_full(Sender *sender){
    return (sender->LAF+1)%SWS==sender->LAR;
}

int sendQ_empty(Sender *sender){
    return sender->LAF == sender->LAR;
}

void print_queue(Sender *sender){
    int max = SWS;
    int start = sender->LAR;
    int end = sender->LAF;

    while(start!=end){
        Frame *ff = sender->sendQ[start].frame;
        char seq_num;
        frame_get_seq_num(ff,&seq_num);
        printf("%d--",seq_num);
        start+=1;
        start%=max;
    }
}


void handle_incoming_acks(Sender * sender,
                          LLnode ** outgoing_frames_head_ptr)
{
    int frame_length = ll_get_length(sender->input_framelist_head);
    while (frame_length > 0)
    {
        //Pop a node off and update the input_cmd_length
        LLnode * ll_frame_node = ll_pop_node(&sender->input_framelist_head);
        frame_length = ll_get_length(sender->input_framelist_head);

        Frame * f = (Frame *) ll_frame_node->value;
        free(ll_frame_node);

        char income_f_type;

        frame_get_type(f,&income_f_type);

        if(income_f_type==0){
            //data
        }else if(income_f_type==1){
            //ack
            char income_ack_num;
            frame_get_ack_num(f,&income_ack_num);

            // if(income_ack_num<sender->seq_num){
            //     return;
            // }

            char income_seq_num;
            frame_get_seq_num(f,&income_seq_num);
            if(sendQ_empty(sender)){
                return ;
            }


            int max = SWS;
            int start = sender->LAR;
            int end = sender->LAF;

            int pop_end = -1;

            while(start!=end){

                Frame *ff = sender->sendQ[start].frame;
                if(!ff){
                    break;
                }
                char seq_num;

                frame_get_seq_num(ff,&seq_num);

                if(seq_num==income_ack_num){
                    pop_end = start;
                    break;
                }
                start+=1;
                start%=max;
            }

            //连续确认
            if(pop_end!=-1){
                start = sender->LAR;
                end = pop_end;

                while(start!=end){

                    Frame *ff = sender->sendQ[start].frame;
                    sender->sendQ[start].frame = NULL;
                    sender->LAR = (sender->LAR+1)%SWS;
                    sender->RWS-=1;

                    free(ff);

                    start+=1;
                    start%=max;
                }

            }



        }else if(income_f_type==2){
            //回退N步
            char income_seq_num;
            frame_get_seq_num(f,&income_seq_num);
            int is_send_start = 0;

            int max = SWS;
            int start = sender->LAR;
            int end = sender->LAF;

            while(start!=end){

                Frame *ff = sender->sendQ[start].frame;
                char seq_num;
                frame_get_seq_num(ff,&seq_num);
//                printf("%d %d\n",income_seq_num,seq_num);
                if(seq_num==income_seq_num){
                    is_send_start = 1;
                }
                if(is_send_start){
                    char * outgoing_charbuf = convert_frame_to_char(ff);
                    frame_add_crc_8(outgoing_charbuf);
                    ll_append_node(outgoing_frames_head_ptr,
                                   outgoing_charbuf);
                }
                start+=1;
                start%=max;
            }


        }
    }
}


void handle_input_cmds(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling input cmd
    //    1) Dequeue the Cmd from sender->input_cmdlist_head
    //    2) Convert to Frame
    //    3) Set up the frame according to the sliding window protocol
    //    4) Compute CRC and add CRC to Frame

    int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    
        
    //Recheck the command queue length to see if stdin_thread dumped a command on us
    input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    while (input_cmd_length > 0)
    {

        if(sendQ_full(sender)){
            // printf("%s\n", "queue full..");
            return ;
        }
    	
        //Pop a node off and update the input_cmd_length
        LLnode * ll_input_cmd_node = ll_pop_node(&sender->input_cmdlist_head);
        input_cmd_length = ll_get_length(sender->input_cmdlist_head);

        //Cast to Cmd type and free up the memory for the node
        Cmd * outgoing_cmd = (Cmd *) ll_input_cmd_node->value;
        free(ll_input_cmd_node);


        //DUMMY CODE: Add the raw char buf to the outgoing_frames list
        //NOTE: You should not blindly send this message out!
        //      Ask yourself: Is this message actually going to the right receiver (recall that default behavior of send is to broadcast to all receivers)?
        //                    Does the receiver have enough space in in it's input queue to handle this message?
        //                    Were the previous messages sent to this receiver ACTUALLY delivered to the receiver?
        int msg_length = strlen(outgoing_cmd->message);
        if (msg_length > MAX_FRAME_SIZE)
        {
            //Do something about messages that exceed the frame size
            printf("<SEND_%d>: sending messages of length greater than %d is not implemented\n", sender->send_id, MAX_FRAME_SIZE);
        }
        else
        {

            //This is probably ONLY one step you want
            Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
            strcpy(outgoing_frame->data, outgoing_cmd->message);


            // ---------2017-12-16------------//
            char dst_id = (char)(outgoing_cmd->dst_id);
            char src_id = (char)(outgoing_cmd->src_id);
            frame_add_dst_src(outgoing_frame,dst_id,src_id);


            unsigned char seq = sender->seq_num;
            frame_add_seq_num(outgoing_frame,seq);

            //正常的数据包类型
            unsigned char f_type = 0;
            frame_add_type(outgoing_frame,f_type);

            //At this point, we don't need the outgoing_cmd
            free(outgoing_cmd->message);
            free(outgoing_cmd);

            //Convert the message to the outgoing_charbuf
            char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);

            // ---------2017-12-15------------//
            frame_add_crc_8(outgoing_charbuf);



            ll_append_node(outgoing_frames_head_ptr,
                           outgoing_charbuf);

            //保留frame，ACK之后再释放
            //free(outgoing_frame);

            // 获取时间
            struct timeval current_time;
            gettimeofday(&current_time,NULL);


            sender->sendQ[sender->LAF].startime.tv_sec = current_time.tv_sec;
            sender->sendQ[sender->LAF].startime.tv_usec = current_time.tv_usec;
            sender->sendQ[sender->LAF].endtime.tv_sec = current_time.tv_sec;
            sender->sendQ[sender->LAF].endtime.tv_usec = current_time.tv_usec+200000;//1s = 1000000


            sender->sendQ[sender->LAF].frame = outgoing_frame;


            //更新sender参数
            sender->RWS+=1;
            sender->LAF=(1+sender->LAF)%SWS;
            sender->seq_num +=1;

        }
    }   
}


void handle_timedout_frames(Sender * sender,
                            LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling timed out datagrams
    //    1) Iterate through the sliding window protocol information you maintain for each receiver
    //    2) Locate frames that are timed out and add them to the outgoing frames
    //    3) Update the next timeout field on the outgoing frames
    //    int length = sender->RWS;

    if(sendQ_empty(sender)){
        return ;
    }
    struct timeval  curr_timeval;

    int max = SWS;
    int start = sender->LAR;
    int end = sender->LAF;

    while(start!=end){
    	gettimeofday(&curr_timeval,NULL);
    	long last_time = timeval_usecdiff(&curr_timeval,&(sender->sendQ[start].endtime));

		if(last_time<0){
		        Frame *f = sender->sendQ[start].frame;

		        char * outgoing_charbuf = convert_frame_to_char(f);
		        ll_append_node(outgoing_frames_head_ptr, outgoing_charbuf);
		        /*添加crc校验*/
		        frame_add_crc_8(outgoing_charbuf);

		        struct timeval current_time;
		        gettimeofday(&current_time,NULL);

		        sender->sendQ[start].startime.tv_sec = current_time.tv_sec;
		        sender->sendQ[start].startime.tv_usec = current_time.tv_usec;
		        sender->sendQ[start].endtime.tv_sec = current_time.tv_sec;
		        sender->sendQ[start].endtime.tv_usec = current_time.tv_usec+200000;//1s = 1000000

		}
        start+=1;
        start%=max;
    }
//     //处理第一个Frame
//     gettimeofday(&curr_timeval,NULL);

//     long last_time = timeval_usecdiff(&curr_timeval,&(sender->sendQ[sender->LAR].endtime));

// //    printf("%ld\n",last_time);

//     if(last_time<0){
//         Frame *f = sender->sendQ[sender->LAR].frame;


//         char * outgoing_charbuf = convert_frame_to_char(f);
//         ll_append_node(outgoing_frames_head_ptr, outgoing_charbuf);
//         /*添加crc校验*/
//         frame_add_crc_8(outgoing_charbuf);

//         struct timeval current_time;
//         gettimeofday(&current_time,NULL);

//         sender->sendQ[sender->LAF].startime.tv_sec = current_time.tv_sec;
//         sender->sendQ[sender->LAF].startime.tv_usec = current_time.tv_usec;
//         sender->sendQ[sender->LAF].endtime.tv_sec = current_time.tv_sec;
//         sender->sendQ[sender->LAF].endtime.tv_usec = current_time.tv_usec+200000;//1s = 1000000

//         sender->sendQ[sender->LAF].frame = f;

//         sender->LAR = (1+sender->LAR)%SWS;
//         sender->LAF = (1+sender->LAF)%SWS;
// //        sender->seq_num +=1;
//     }

}


void * run_sender(void * input_sender)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Sender * sender = (Sender *) input_sender;    
    LLnode * outgoing_frames_head;
    struct timeval * expiring_timeval;
    long sleep_usec_time, sleep_sec_time;
    
    //This incomplete sender thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up
    //2. Grab the mutex protecting the input_cmd/inframe queues
    //3. Dequeues messages from the input queue and adds them to the outgoing_frames list
    //4. Releases the lock
    //5. Sends out the messages

    pthread_cond_init(&sender->buffer_cv, NULL);
    pthread_mutex_init(&sender->buffer_mutex, NULL);

    while(1)
    {  
      
        outgoing_frames_head = NULL;

        //Get the current time
        gettimeofday(&curr_timeval, 
                     NULL);

        //time_spec is a data structure used to specify when the thread should wake up
        //The time is specified as an ABSOLUTE (meaning, conceptually, you specify 9/23/2010 @ 1pm, wakeup)
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;

        //Check for the next event we should handle
        expiring_timeval = sender_get_next_expiring_timeval(sender);

        //Perform full on timeout
        if (expiring_timeval == NULL)
        {
            time_spec.tv_sec += WAIT_SEC_TIME;
            time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        }
        else
        {
            //Take the difference between the next event and the current time
            //睡到检查时间
            sleep_usec_time = timeval_usecdiff(&curr_timeval,
                                               expiring_timeval);

            //Sleep if the difference is positive
            if (sleep_usec_time > 0)
            {
                sleep_sec_time = sleep_usec_time/1000000;
                sleep_usec_time = sleep_usec_time % 1000000;   
                time_spec.tv_sec += sleep_sec_time;
                time_spec.tv_nsec += sleep_usec_time*1000;
            }   
        }

        //Check to make sure we didn't "overflow" the nanosecond field
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        
        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames or input commands should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&sender->buffer_mutex);

        //Check whether anything has arrived
        int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        int inframe_queue_length = ll_get_length(sender->input_framelist_head);
        
        //Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
        //A signal on the condition variable will wakeup the thread and reaquire the lock
        if (input_cmd_length == 0 &&
            inframe_queue_length == 0)
        {
            
            pthread_cond_timedwait(&sender->buffer_cv, 
                                   &sender->buffer_mutex,
                                   &time_spec);
        }




        
        //Implement this
        handle_incoming_acks(sender,
                             &outgoing_frames_head);

        //Implement this
        handle_input_cmds(sender,
                          &outgoing_frames_head);


        pthread_mutex_unlock(&sender->buffer_mutex);


        //Implement this
        handle_timedout_frames(sender,
                               &outgoing_frames_head);

        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames







        //发送所有帧
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *)  ll_outframe_node->value;

            //Don't worry about freeing the char_buf, the following function does that
            send_msg_to_receivers(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);
    return 0;
}
