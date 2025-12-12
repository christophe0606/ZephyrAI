/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "stream.h"



int main(void)
{   
	printf("Starting main\n");


	int err = init_stream();
	if (err != 0)
	{
		printf("Error initializing stream\n");
		return -1;
	}
	
	return 0;

}
