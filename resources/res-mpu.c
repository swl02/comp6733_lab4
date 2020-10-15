/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"
#include <limits.h>

#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "board-peripherals.h"
#define CHUNKS_TOTAL    2050


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
char type[10];
static struct ctimer mpu_timer;
int counter = 0;
char out_str[1000];

/*
 * Example for a resource that also handles all its sub-resources.
 * Use REST.get_url() to multiplex the handling of the request depending on the Uri-Path.
 */
PARENT_RESOURCE(res_mpu,
                "title=\"Sub-resource demo\"",
                res_get_handler,
                NULL,
                NULL,
                NULL);



static void gyro_init(void *n) {
  mpu_9250_sensor.configure(SENSORS_ACTIVE,MPU_9250_SENSOR_TYPE_GYRO_ALL);
  int val = 0;
  if (counter < (int)n) {
	  ctimer_reset(&mpu_timer);
    if (strncmp(type,"x",1) == 0) {
      val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
    } else if (strncmp(type,"y",1) == 0) {
      val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
    } else if (strncmp(type,"z",1) == 0) {
      val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
    }

    sprintf(out_str,REST_MAX_CHUNK_SIZE,"The gyro %c value is %d ./s\n",(char)type[0],val);


	  counter++;

  } else {
	  counter = 0;
	  ctimer_stop(&mpu_timer);
  }


}


static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

  const char *uri_path = NULL;
  int len = REST.get_url(request, &uri_path);
  int base_len = strlen(res_mpu.url);


  if(len == base_len) {
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Request any sub-resource of /%s", res_mpu.url);
  } else {
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, ".%.*s", len - base_len, uri_path + base_len);
  }

  char * token = strtok(buffer,"/");
  int stage = 0;
  int n = 0;
  while (token != NULL) {
    if (stage == 1) {
      strcpy(type,token);
    }

    if (stage == 2) {
      n = atoi(token);
    }



    token = strtok(NULL,"/");
    stage ++;
  }

  ctimer_set(&mpu_timer, CLOCK_SECOND, gyro_init, (void *)n);	

  // int32_t strpos = 0;

  // /* Check the offset for boundaries of the resource data. */
  // if(*offset >= CHUNKS_TOTAL) {
  //   REST.set_response_status(response, REST.status.BAD_OPTION);
  //   /* A block error message should not exceed the minimum block size (16). */

  //   const char *error_msg = "BlockOutOfScope";
  //   REST.set_response_payload(response, error_msg, strlen(error_msg));
  //   return;
  // }

  // /* Generate data until reaching CHUNKS_TOTAL. */
  // while(strpos < preferred_size) {
  //   strpos += snprintf((char *)out_str + strpos, preferred_size - strpos + 1, "%s", (char *)out_str);
  // }

  // /* snprintf() does not adjust return value if truncated by size. */
  // if(strpos > preferred_size) {
  //   strpos = preferred_size;
  //   /* Truncate if above CHUNKS_TOTAL bytes. */
  // }
  // if(*offset + (int32_t)strpos > CHUNKS_TOTAL) {
  //   strpos = CHUNKS_TOTAL - *offset;
  // }
  // REST.set_response_payload(response, out_str, strpos);

  // /* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
  // *offset += strpos;

  // /* Signal end of resource representation. */
  // if(*offset >= CHUNKS_TOTAL) {
  //   *offset = -1;
  // }







  REST.set_response_payload(response, out_str, strlen((char *)out_str));
}



// static void
// res_periodic_handler()
// {
//   mpu_9250_sensor.configure(SENSORS_ACTIVE,MPU_9250_SENSOR_TYPE_GYRO_ALL);
//     int val = 0;
//     if (strncmp(type,"x",1) == 0) {
//       val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
//     } else if (strncmp(type,"y",1) == 0) {
//       val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
//     } else if (strncmp(type,"z",1) == 0) {
//       val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
//     }
  


//   ++interval_counter;

//   if((abs(val - val_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
//      interval_counter >= INTERVAL_MAX) {
//      interval_counter = 0;
//      val_old = val;
//     /* Notify the registered observers which will trigger the res_get_handler to create the response. */
//     REST.notify_subscribers(&res_mpu);
//   }
// }

