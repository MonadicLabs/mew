
#include "utils.h"

#include <uv.h>

double hrtime()
{
    double gt = (double)uv_hrtime() / (double)(1000.0*1000.0*1000.0);
    return gt;
}
