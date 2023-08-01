// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef pqCoreInit_h
#define pqCoreInit_h

#include "pqCoreModule.h"

/**
 * pqCoreInit() is for initializing QT resources. When pqCore is built
 * as a static library, linkers may drop out entire object
 * files such as files generated by rcc. Client apps can call to use Qt
 * resources outside PV.
 */

PQCORE_EXPORT void pqCoreInit();

#endif
