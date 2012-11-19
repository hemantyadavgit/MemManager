/***********************
Author: Hemant Yadav
***********************/
#include <stdio.h>

#define MaxPackets 10
#define PKT_DATA_SIZE 8

typedef char data_t[PKT_DATA_SIZE];

/* structure sent to our "receiver" */
typedef struct {
  int how_many; /* number of packets in the message */
  int which;
  data_t data;  /* packet data */
} packet_t;

typedef struct
{
	unsigned short int received;
	data_t data;
} pktfrag_t;	

typedef struct 
{
  int num_packets;
  pktfrag_t * fragments;		// eventually points into mm chunk to array of pktfrag_t's
} message_t;

/* print_message: prints data_t array of entire message */
/* data is displayed as a character array */
void print_message(message_t * pmess);
