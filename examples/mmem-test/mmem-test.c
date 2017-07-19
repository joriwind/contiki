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

#define NUM_CHUNKS 8
#define CHUNK_SIZE 50

#define USE_SEQUENCE

#define SECONDS 2
#define CHUNK_GROWTH cycles

#ifdef USE_SEQUENCE
#define FREE_SEQUENCE {7, 1, 3, 5, 0, 2, 6, 4}
#define OFFSET 0
#else
#define OFFSET 4
#define FREE_SEQUENCE 0
#endif

uint32_t cycles;

static struct mmem memlist[NUM_CHUNKS];
static bool active[NUM_CHUNKS];

void * calloc_func(struct mmem *list, uint8_t i);
void free_func(struct mmem *list, uint8_t i);

void * calloc_func(struct mmem *list, uint8_t i){
  if(!active[i]){
    if (!mmem_alloc(&list[i], CHUNK_SIZE + CHUNK_GROWTH)){
      printf("mmem_alloc failed\n");
      return NULL;
    }else{
      active[i] = true;
      //printf("C%u ", i);
      return MMEM_PTR(&list[i]);
    }
  }
  printf("No objects available\n");
  return NULL;    
    
}

void free_func(struct mmem *list, uint8_t i){
  if(active[i]){
    mmem_free(&list[i]);
    active[i] = false;
    //printf("F%u ", i);
  }
}


/*---------------------------------------------------------------------------*/
PROCESS(mmem_inorder_tester, "MMEM free in order");
PROCESS(profiler, "profiler");
AUTOSTART_PROCESSES(&profiler);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mmem_inorder_tester, ev, data)
{
  uint8_t i;
  #ifndef USE_SEQUENCE
  uint8_t j;
  #endif

  PROCESS_BEGIN();


  cycles = 0;
  printf("Starting MMEM inorder test\n");

  while (1) {
    /* Allocate the memory... */
    
    printf(" %u ", CHUNK_GROWTH);
    for (i=0;i<NUM_CHUNKS;i++) {
      if(!calloc_func(memlist, i)){
        goto errorReturn;
      }
    }
 


    /* ...and free it again */
    #ifdef USE_SEQUENCE
    uint8_t sequence[NUM_CHUNKS] = FREE_SEQUENCE;
    for (i = 0; i < NUM_CHUNKS; i++){
      free_func(memlist, sequence[i]);
    }
    #else
    for (i = OFFSET;i<NUM_CHUNKS + OFFSET;i++) {
      j = i % NUM_CHUNKS;
      free_func(memlist, j);
    }
    #endif

    cycles += 1;

    /* if (cycles == 20){
      return 0;
    } */

    PROCESS_PAUSE();
  }

 errorReturn:
  printf("Capture error, from %u\n", i - 1);
  for (i = 0;i < NUM_CHUNKS;i++) {
    free_func(memlist, i);
  } 

  //PROCESS_PAUSE();

  PROCESS_END();
}

PROCESS_THREAD(profiler, ev, data)
{
	static struct etimer timer;
	PROCESS_BEGIN();

	mmem_init();

  printf("Starting mmem tests: interval: %u offset: %u chunk-growth: %u, sequence: ", SECONDS, OFFSET, CHUNK_GROWTH);
  #ifdef USE_SEQUENCE
  printf("{");
  uint8_t sequence[NUM_CHUNKS] = FREE_SEQUENCE;
  int i;
  for(i = 0; i<NUM_CHUNKS;i++){
    printf("%u, ", sequence[i]);
  }
  printf("}");
  #endif
  printf("\n");

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

  printf("End of tests");

	PROCESS_END();
}
