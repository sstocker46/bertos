/*!
 * \file
 * <!--
 * Copyright 2004 Develer S.r.l. (http://www.develer.com/)
 * Copyright 1999,2000,2001 Bernardo Innocenti <bernie@develer.com>
 * All Rights Reserved.
 * -->
 *
 * \brief Heap subsystem (public interface).
 *
 * \version $Id$
 *
 * \author Bernardo Innocenti <bernie@develer.com>
 */

/*
 * $Log$
 * Revision 1.1  2004/05/23 17:27:00  bernie
 * Import kern/ subdirectory.
 *
 */

#ifndef KERN_HEAP_H
#define KERN_HEAP_H

#include "compiler.h"


/* Memory allocation services */
void heap_init(void);
void *heap_alloc(size_t size);
void heap_free(void *mem, size_t size);

#ifdef __POSIX__ /* unused */
void *malloc(size_t size);
void *calloc(unsigned int nelem, size_t size);
void free(void * mem);
#endif /* __POSIX__ */

#endif /* KERN_HEAP_H */

