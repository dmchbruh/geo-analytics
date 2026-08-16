#include "h3/HexMetrics.h"

#include <h3/h3api.h>

double getHexAreaKm2(H3Index hexId)
{
    if (!isValidCell(hexId))
    {
        return 0.0;
    }

    double area = 0.0;

    H3Error error = cellAreaKm2(hexId, &area);

    if (error != E_SUCCESS)
    {
        return 0.0;
    }

    return area;
}