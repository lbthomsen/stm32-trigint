//
//  benchmark.h
//  sin16
//
//  Created by Dave Dribin on 4/27/10.
//  Copyright 2010 Bit Maki, Inc. All rights reserved.
//


void ddprintf(NSString * format, ...);
void dd_sin16_bench(uint32_t iters);
void lib_sin_bench(uint32_t iters);
void lib_sinf_bench(uint32_t iters);

void bench_check_error();
