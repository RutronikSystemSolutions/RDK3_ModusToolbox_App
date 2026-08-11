/*
 * i2c_mutex.c
 *
 *  Created on: 15.07.2026
 *      Author: DC1
 */


#include "i2c_mutex.h"

static volatile bool i2c_locked = false;

void i2c_mutex_init(void)
{
    uint32_t key = Cy_SysLib_EnterCriticalSection();
    i2c_locked = false;
    Cy_SysLib_ExitCriticalSection(key);
}

static inline bool lock_try(void)
{
    if (!i2c_locked)
    {
        i2c_locked = true;
        return true;
    }
    return false;
}

void i2c_mutex_take(void)
{
    for (;;)
    {
        uint32_t key = Cy_SysLib_EnterCriticalSection();
        bool ok = lock_try();
        Cy_SysLib_ExitCriticalSection(key);
        if (ok) break;
    }
}

bool i2c_mutex_try_take(void)
{
    uint32_t key = Cy_SysLib_EnterCriticalSection();
    bool ok = lock_try();
    Cy_SysLib_ExitCriticalSection(key);
    return ok;
}

void i2c_mutex_give(void)
{
    uint32_t key = Cy_SysLib_EnterCriticalSection();
    i2c_locked = false;
    Cy_SysLib_ExitCriticalSection(key);
}