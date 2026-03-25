#include "WestPatchLane.h"

void WestPatchLane::prepare (double sampleRate)
{
    carrierOsc.prepare (sampleRate);
    modOsc.prepare (sampleRate);
    lpg.prepare (sampleRate);
}
