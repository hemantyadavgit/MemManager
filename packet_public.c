/***********************
Author: Hemant Yadav
***********************/
#include "packet_public.h"
#include "mm_public.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MM_SIZE 200

message_t message;     /* current message structure */
mm_t MM;               /* memory manager will allocate memory for packets */
int pkt_cnt;	       /* how many packets have arrived for current message */
int pkt_total = 1;     /* how many packets to be received for the message */
int NumMessages = 5;   /* number of messages we will receive */
int cnt_msg=1;         /* current message being received*/

packet_t get_packet (int size) {
  packet_t pkt;
  static int which;

  pkt.how_many = size;
  which=rand()%pkt.how_many; //the order in which packets will be sent is random.
  pkt.which = which;
  if (which == 0)
   	strcpy (pkt.data, "aaaaaaa\0");
  else if (which == 1)
    strcpy (pkt.data, "bbbbbbb\0");
  else if(which == 2)
    strcpy (pkt.data, "ccccccc\0");
  else if(which == 3)
    strcpy (pkt.data, "ddddddd\0");
  else
    strcpy (pkt.data, "eeeeeee\0");
 return pkt;
}


void packet_handler(int sig)
{
  fflush(stdout);
  packet_t pkt;

  /*  fprintf (stderr, "IN PACKET HANDLER, sig=%d\n", sig); */
  pkt = get_packet(cnt_msg); //the messages are of variable length. So, the 1st message consists of 1 packet, the 2nd message consists of 2 packets and so on..
  if(pkt_cnt==0) //when the 1st packet arrives, the size of the whole message is allocated.
  {
     int i;
     if ((message.fragments = (pktfrag_t *)mm_get(&MM,pkt.how_many*sizeof(pktfrag_t))) == NULL)
     {				// attempt to grab space for how_many pktfrag_t structs
	fprintf(stderr,"%s\n","unable to grab space for message");
        return;
     }
     
     for(i=0;i<pkt.how_many;i++)			// now set all pktfrag_t structs within this chunk as unreceived
       message.fragments[i].received = 0;

     pkt_total = pkt.how_many;				// initialize message metadata
     message.num_packets = pkt.how_many;
  }
  if (message.fragments[pkt.which].received == 0)			// this is a new packet
  {
    memcpy(message.fragments[pkt.which].data,pkt.data,sizeof(data_t));	// copy in packet to correct location
    message.fragments[pkt.which].received = 1;				// indicate this packet has been received
    pkt_cnt++;
  }
}


int main (int argc, char **argv)
{         

  // set up SIGALRM handler
  struct sigaction act;
  act.sa_sigaction = NULL;
  act.sa_handler = packet_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask,SIGALRM); // block alarms within ALRM handler
  if (sigaction(SIGALRM, &act, NULL) < 0)
  {
    perror("signal handler");
    exit(EXIT_FAILURE);
  }

  // set up SIGALRM timer
  struct itimerval itimestruct;
  struct timeval timestruct;
  timestruct.tv_sec = INTERVAL;
  timestruct.tv_usec = INTERVAL_USEC;
  itimestruct.it_interval = timestruct;
  itimestruct.it_value = timestruct;
  if (setitimer(ITIMER_REAL,&itimestruct,NULL) == -1)
  {
    perror("alarm timer");
    exit(EXIT_FAILURE);
  }

  if (mm_init (&MM, MM_SIZE) == -1)
  {
    fprintf(stderr,"%s\n","error initializing mm");
    exit(EXIT_FAILURE);
  }

  for (cnt_msg=1; cnt_msg<=NumMessages; cnt_msg++)
  {
    pkt_cnt = 0;
    pause();			// wait for first packet.  Will update message structure, global variable pkt_total
    for(pkt_cnt=1;pkt_cnt<pkt_total;)
      pause();			// wait for signal that packet has arrived
    print_message(&message);
    mm_put(&MM,message.fragments);
  }
  
  // now disable timer.  If timer squeezes in with another alarm, it will be no big deal; the NumMessages'th message will just be partially recopied
  timestruct.tv_sec = 0;		// value of 0 disables the timer
  timestruct.tv_usec = 0;
  itimestruct.it_interval = timestruct;
  itimestruct.it_value = timestruct;
  if (setitimer(ITIMER_REAL,&itimestruct,NULL) == -1)
  {
    perror("alarm timer");
    exit(EXIT_FAILURE);
  }

  mm_release(&MM);

  printf("\n");
}

void print_message(message_t * pmess)
{
	printf("Message: ");
	int i;
	for(i=0;i<pmess->num_packets;i++)
	{
		int j;
		for(j=0;j<sizeof(data_t);j++)
			printf("%c",pmess->fragments[i].data[j]);
	}
	printf("\n");
}

