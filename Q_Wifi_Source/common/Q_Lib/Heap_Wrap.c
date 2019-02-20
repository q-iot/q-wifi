#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espressif/esp8266/ets_sys.h"
#include "esp_libc.h"
#include "Q_Heap.h"

#if USE_Q_HEAP //为了与系统heap5并存
size_t xPortGetFreeHeapSize( void )
{
	return QHeapGetIdleSize();
}

size_t xPortWantedSizeAlign(size_t xWantedSize)
{
	return xWantedSize;
}

#ifdef MEMLEAK_DEBUG
void *pvPortMalloc( size_t xSize, const char *file, unsigned line, bool use_iram)
{
#if Q_HEAP_DEBUG > 1
	return _Q_Malloc(xSize,file,line);
#else	
	return _Q_Malloc(xSize);
#endif
}

void vPortFree( void *pv, const char * file, unsigned line)
{
#if Q_HEAP_DEBUG > 1
	return _Q_Free(pv,file,line);
#else
	return _Q_Free(pv);
#endif
}

void *pvPortCalloc(size_t count, size_t size, const char * file, unsigned line)
{
	void *p;
	p = pvPortMalloc(count * size, file, line, false);
	return p;
}

void *pvPortZalloc(size_t size, const char * file, unsigned line)
{
    return pvPortMalloc(size, file, line,false);
}

void *pvPortRealloc(void *mem, size_t newsize, const char *file, unsigned line)
{
    if (newsize == 0) {
        vPortFree(mem, file, line);
        return NULL;
    }

    void *p;
    p = pvPortMalloc(newsize, file, line, false);
    if (p) {
        if (mem != NULL) {
            memcpy(p, mem, newsize);
            vPortFree(mem, file, line);
        }
    }
    return p;
}
#else
void *pvPortMalloc( size_t xSize)
{
#if Q_HEAP_DEBUG > 1
	return _Q_Malloc(xSize,__FUNCTION__,0);
#else	
	return _Q_Malloc(xSize);
#endif
}

void vPortFree( void *pv)
{
#if Q_HEAP_DEBUG > 1
	return _Q_Free(pv,__FUNCTION__,0);
#else
	return _Q_Free(pv);
#endif
}

void *pvPortCalloc(size_t count, size_t size)
{
	void *p;
	p = pvPortMalloc(count * size);
	return p;
}

void *pvPortZalloc(size_t size)
{
    return pvPortMalloc(size);
}

void *pvPortRealloc(void *mem, size_t newsize)
{
    if (newsize == 0) {
        vPortFree(mem);
        return NULL;
    }

    void *p;
    p = pvPortMalloc(newsize);
    if (p) {
        if (mem != NULL) {
            memcpy(p, mem, newsize);
            vPortFree(mem);
        }
    }
    return p;
}
#endif

void *malloc(size_t nbytes)
{
	return pvPortMalloc(nbytes, __FUNCTION__, 0, false);
}

void free(void *ptr)
{
	vPortFree(ptr, __FUNCTION__, 0);
}

void *zalloc(size_t nbytes)
{
	return pvPortZalloc(nbytes, __FUNCTION__, 0);
}

void *calloc(size_t count, size_t nbytes)
{
	return pvPortCalloc(count, nbytes, __FUNCTION__, 0);
}

void *realloc(void *ptr, size_t nbytes)
{
	return pvPortRealloc(ptr, nbytes, __FUNCTION__, 0);
}
#endif

