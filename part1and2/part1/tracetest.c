/*
 * File        : tracetest.c
 * Author      : Cory Agami / Brandom Hillan(?)
 * Class       : Operating Systems, Project 2, Part 1
 *
 * Description : Program that currently executes 42 system calls, 
 *               which is exactly 20 more than the empty program (22).
 */

#include <stdio.h>

int main()
{
	
	FILE *fp;
	fp = fopen("temp.txt", "w+");
	fprintf(fp, "This is testing for fprintf...\n");
	fputs("This is testing for fputs...\n", fp);
	fclose(fp);
	
	system("rm temp.txt");
	
	sleep(1);

	return  0;
}
