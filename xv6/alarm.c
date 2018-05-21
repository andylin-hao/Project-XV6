//
// Created by masteryoda on 18-5-21.
//
#include "types.h"
#include "stat.h"
#include "user.h"

void periodic();

int main(int argc, char *argv[])
{
    int i;
    printf(1, "alarmtest starting\n");
    alarm(10, periodic);
    while (1)
    {
        if ((i % 250000) == 0)
            printf(1, ".", 1);
        i++;
    }
    exit();
}

void periodic()
{
    printf(1, "alarm!\n");
}
