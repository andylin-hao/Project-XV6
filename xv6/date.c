#include "types.h"
#include "user.h"
#include "date.h"

int main(int argc, char *argv[])
{
	struct rtcdate timeTeller;

	if(date(&timeTeller))
	{
		printf(1,"process failed\n");
		exit();
	}
	printf(1, "%d-%d-%d %d:%d:%d\n", timeTeller.year, timeTeller.month, timeTeller.day, timeTeller.hour, timeTeller.minute, timeTeller.second);
	exit();

}