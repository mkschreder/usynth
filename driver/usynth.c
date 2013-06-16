#define _POSIX_C_SOURCE 199309L
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include "usynth.h"

typedef struct usynth_interface_s{
	int 	serial_port; 
} usynth_interface_t; 

static const char *PORT_NAME = "/dev/ttyUSB0";
struct termios options_original;

static int SER_Init(){
	struct termios options;

  int serial_port = open(PORT_NAME, O_RDWR);

  if (serial_port != -1)
  {
	  printf("Serial Port open\n");
	  tcgetattr(serial_port,&options_original);
 	  tcgetattr(serial_port, &options);
	  cfsetispeed(&options, B9600);
	  cfsetospeed(&options, B9600);
	  
	  options.c_cflag &= ~PARENB;  // N
    options.c_cflag &= ~CSTOPB;  // 1
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;      // 8
    
    //options.c_cflag |= CRTSCTS;   // no flow control
    options.c_cflag |= CREAD | CLOCAL;  // turn on read & ignore ctrl lines
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    //options.c_oflag &= ~OPOST; // make raw
    
	  tcsetattr(serial_port, TCSANOW, &options);
  }
  else
	  printf("Unable to open /dev/ttyUSB0\n");
  return (serial_port);
}


static void SER_Close(int port)
{
	tcsetattr(port,TCSANOW,&options_original);
	close(port);
}

static int SER_Write(int port, const char *write_buffer, size_t size){
	int bytes_written;
	bytes_written = write(port, write_buffer, size);
	if (bytes_written < size)
	{
		printf("Write failed \n");
	}
	return bytes_written;
}

static int SER_Read(int port, char *read_buffer, size_t max_chars_to_read)
{
	int chars_read = read(port, read_buffer, max_chars_to_read);

	return chars_read;
}

handle_t U_Open(){
	usynth_interface_t *synth = malloc(sizeof(usynth_interface_t)); 
	synth->serial_port = SER_Init(); 
	return synth; 
}

void U_Close(handle_t h){
	usynth_interface_t *synth = (usynth_interface_t*)h; 
	free(synth);
}

void U_NoteOn(handle_t h, uint8_t note, uint8_t velocity){
	usynth_interface_t *synth = (usynth_interface_t*)h; 
	char buf[3]; 
	buf[0] = '\x80';
	buf[1] = note;
	buf[2] = velocity;
	SER_Write(synth->serial_port, buf, 3);
}

void U_NoteOff(handle_t h, uint8_t note, uint8_t velocity){
	usynth_interface_t *synth = (usynth_interface_t*)h; 
	char buf[3]; 
	buf[0] = '\x90';
	buf[1] = note;
	buf[2] = velocity;
	SER_Write(synth->serial_port, buf, 3);
}

void U_SetKnob(handle_t h, knob_t knob, int8_t value){
	usynth_interface_t *synth = (usynth_interface_t*)h; 
	char buf[3]; 
	buf[0] = '\xb0';
	buf[1] = knob; 
	buf[2] = value; 
	SER_Write(synth->serial_port, buf, 3); 
}
