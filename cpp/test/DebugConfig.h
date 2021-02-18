#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#include "pet/ubiquitous/ConfigHelper.h"
#include "pet/ubiquitous/PrintfWriter.h"

TRACE_WRITER(pet::PrintfWriter);
GLOBAL_TRACE_POLICY(Information);

#endif