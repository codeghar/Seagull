/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * (c)Copyright 2006 Hewlett-Packard Development Company, LP.
 *
 */

#if defined(__hpux)
#ifdef USE_DLOPEN
#include <dlfcn.h>
#else

#include <dl.h>
#include <errno.h>

#define RTLD_LAZY BIND_IMMEDIATE

#define dlopen(path,flags) shl_load((path), (flags) | DYNAMIC_PATH, 0L)
#define dlclose(handle)    shl_unload((handle))
#define dlerror()          strerror(errno)
void* dlsym(void* handle, const char *name) { 
  
  void *value;
  
  if ( shl_findsym((shl_t *)&handle, name, TYPE_PROCEDURE, &value) < 0 )
    return NULL;
  
  return value;
}
#endif 

#else

#include <dlfcn.h>

#endif





