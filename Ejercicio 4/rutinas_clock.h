#ifndef RUTINAS_CLOCK_H
#define RUTINAS_CLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

void access_counter();
void start_counter();
double get_counter();
double mhz();

#endif	// RUTINAS_CLOCK_H

