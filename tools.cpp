#include "tools.h"

#define TERABYTE_MULTIPLIER	1099511627776ll
#define GIGABYTE_MULTIPLIER 1073741824
#define MEGABYTE_MULTIPLIER 1048576
#define KILOBYTE_MULTIPLIER 1024

QString Tools::humanReadableBytes(quint64 bytes)
{
	if (bytes >= TERABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)TERABYTE_MULTIPLIER, 'f', 1) + " TiB";
	else if (bytes >= GIGABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)GIGABYTE_MULTIPLIER, 'f', 1) + " GiB";
	else if (bytes >= MEGABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)MEGABYTE_MULTIPLIER, 'f', 1) + " MiB";
	else if (bytes >= KILOBYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)KILOBYTE_MULTIPLIER, 'f', 1) + " KiB";
	else
		return QString::number(bytes) + " bytes";
}
