/***********************************************************************
* Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
* Copyright (C) 2001 by New Riders Publishing                          *
* See COPYRIGHT for license information.                               *
***********************************************************************/

/* Returns the clock speed of the system's CPU in MHz, as reported by
   /proc/cpuinfo.  On a multiprocessor machine, returns the speed of
   the first CPU.  On error returns zero.  */
#include "getcpuclockspeed.h"

long double get_cpu_clock_speed ()
{
  FILE* fp;
  char buffer[1024*1024];
  size_t bytes_read;
  char* match;
  long double clock_speed;

  /* Read the entire contents of /proc/cpuinfo into the buffer.  */

  fp = fopen ("/proc/cpuinfo", "r");
  //just default to 2.7GHz
  if(fp == NULL)
  	return 2.659998;


  bytes_read = fread (buffer, 1, sizeof (buffer), fp);
  fclose (fp);
  /* Bail if read failed or if buffer isn't big enough.  */
  if (bytes_read == 0 || bytes_read == sizeof (buffer))
    return 0;
  /* NUL-terminate the text.  */
  buffer[bytes_read] = '\0';
  /* Locate the line that starts with "cpu MHz".  */
  match = strstr (buffer, "cpu MHz");
  if (match == NULL)
    return 0;
  /* Parse the line to extrace the clock speed.  */
  sscanf (match, "cpu MHz : %Lf", &clock_speed);
  return (long double)clock_speed/1000.0;
}