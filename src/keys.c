#include "keys.h"
#include <ctype.h>

key keys[MAX_KEYS];

int Keys_StringToKeyNum(char *str)
{
	if (!str[1])
	{
		return tolower(str[0]);
	}

	return -1;
}

void Keys_Bind_f()
{
	if (Command_Argc() < 3)
	{
		Print("Usage : bind <key> <cmd>");
	}

	int key = Keys_StringToKeyNum(Command_Argv(1));

	if (key == -1)
	{
		Print("%s is a invalid key", Command_Argv(1));
		return;
	}

	keys[key].binding = Command_Argv(2);
}

void Keys_Unbindall_f()
{
	for (int i = 0; i < MAX_KEYS; i++)
	{
		if (keys[i].binding)
		{
			keys[i].binding = NULL;
		}
	}
}

void Keys_Bindlist_f()
{
	for (int i = 0; i < MAX_KEYS; i++)
	{
		if (keys[i].binding)
		{
			Print("%c : %s", i, keys[i].binding);
		}
	}
}