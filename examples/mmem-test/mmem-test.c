/*
 * Copyright (c) 2012, Daniel Willmann <daniel@totalueberwachung.de>
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
 *
 * $Id:$
 */

/**
 * \file
 *         A program profiling various mmem usage scenarios
 * \author
 *         Daniel Willmann <daniel@totalueberwachung.de>
 */

#include "contiki.h"
#include "lib/mmem.h"

#include <stdio.h>
#include <stdbool.h>

#define NUM_CHUNKS 5
#define CHUNK_SIZE 50


#define OFFSET 4
#define SECONDS 2
#define CHUNK_GROWTH cycles

uint32_t cycles;

/*---------------------------------------------------------------------------*/
PROCESS(mmem_inorder_tester, "MMEM free in order");
PROCESS(mmem_revorder_tester, "MMEM free reverse order");
PROCESS(profiler, "profiler");
AUTOSTART_PROCESSES(&profiler);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mmem_inorder_tester, ev, data)
{
  uint8_t i;
  uint8_t j;
  static struct mmem memlist[NUM_CHUNKS];
  static bool active[NUM_CHUNKS];

  PROCESS_BEGIN();


  cycles = 0;
  printf("Starting MMEM inorder test\n");

  while (1) {
    /* Allocate the memory... */
    printf(" %u ", CHUNK_GROWTH);
    for (i=0;i<NUM_CHUNKS;i++) {
      if(!active[i]){
        if (!mmem_alloc(&memlist[i], CHUNK_SIZE + CHUNK_GROWTH)){
          printf("mmem_alloc failed\n");
          goto errorReturn;
        }else{
          active[i] = true;
        }
      }
        
    }
 


    /* ...and free it again */
    for (i = OFFSET;i<NUM_CHUNKS + OFFSET;i++) {
      j = i % NUM_CHUNKS;
      if(active[j]){
        mmem_free(&memlist[j]);
        active[j] = false;
      }

    }

    cycles += 1;

    /* if (cycles == 20){
      return 0;
    } */

    PROCESS_PAUSE();
  }

 errorReturn:
  printf("Capture error, from %u\n", i - 1);
  for (i = 0;i < NUM_CHUNKS;i++) {
    if(active[i]){
      mmem_free(&memlist[i]);
      active[i] = false;
    }
  } 

  //PROCESS_PAUSE();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mmem_revorder_tester, ev, data)
{
  uint8_t i;
  uint8_t j;
  static struct mmem memlist[NUM_CHUNKS];
  static bool active[NUM_CHUNKS];

  PROCESS_BEGIN();

  cycles = 0;
  printf("Starting MMEM reverse order test\n");

  while (1) {
    printf(" %u ", CHUNK_GROWTH);
    /* Allocate the memory... */
    for (i=0;i<NUM_CHUNKS;i++) {
      if(!active[i]){
        if (!mmem_alloc(&memlist[i], CHUNK_SIZE + CHUNK_GROWTH)){
          printf("mmem_alloc failed\n");
          goto errorReturn;
        }else{
          active[i] = true;
        }
      }
    }

    /* ...and free it again - this time in reverse */
    for (i=NUM_CHUNKS + OFFSET;i > 0 + OFFSET;i--) {
      j = (i - 1) % NUM_CHUNKS;
      if(active[j]){
        mmem_free(&memlist[j]);
        active[j] = false;
      }

    }
    cycles += 1;

    PROCESS_PAUSE();
  }

errorReturn:
  printf("Capture error, from %u\n", i - 1);
  for (i = 0;i < NUM_CHUNKS;i++) {
    if(active[i]){
      mmem_free(&memlist[i]);
      active[i] = false;
    }
  } 

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(profiler, ev, data)
{
	static struct etimer timer;
	PROCESS_BEGIN();

	mmem_init();
  printf("Starting mmem tests: interval: %u offset: %u chunk-growth: %u\n", SECONDS, OFFSET, CHUNK_GROWTH);

	process_start(&mmem_inorder_tester, NULL);
	etimer_set(&timer, CLOCK_SECOND * SECONDS);
	PROCESS_WAIT_UNTIL(etimer_expired(&timer));
	process_exit(&mmem_inorder_tester);
  printf("mmem-inorder: cycles: %u\n", cycles);
	

  process_start(&mmem_inorder_tester, NULL);
	etimer_set(&timer, CLOCK_SECOND * SECONDS);
	PROCESS_WAIT_UNTIL(etimer_expired(&timer));
	process_exit(&mmem_inorder_tester);
  printf("mmem-inorder: cycles: %u\n", cycles);

  
 	 process_start(&mmem_revorder_tester, NULL);
	etimer_set(&timer, CLOCK_SECOND * SECONDS);
	PROCESS_WAIT_UNTIL(etimer_expired(&timer));
	process_exit(&mmem_revorder_tester);
	printf("mmem-revorder: %u\n", cycles);  

  printf("End of tests");

	PROCESS_END();
}
