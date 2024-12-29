#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#define BUF_INC 16

typedef struct {
    char* buffer;      /* Pointer to the dynamic buffer */
    size_t size;       /* Current size of the buffer */
    size_t capacity;   /* Allocated capacity */
} memstream_t;

#ifndef LIBSOL10_COMPAT_HAVE_GETDELIM
ssize_t getdelim(char **lineptr, size_t *n, int delimiter, FILE *restrict fp)
{
	char *cur_pos, *new_lineptr;
	size_t new_lineptr_len;
	int c;

	if (lineptr == NULL || n == NULL || fp == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (*lineptr == NULL) {
		*lineptr = malloc(BUF_INC);
		if (*lineptr  == NULL) {
		    errno = ENOMEM;
		    return -1;
		}
		*n = BUF_INC;
	}

	cur_pos = *lineptr;
	for (;;) {
		c = getc(fp);
		if (ferror(fp) || (c == EOF && cur_pos == *lineptr)) {
		    *cur_pos = '\0';
		    return -1;
		}

		if (c == EOF) {
		    break;
		}

		if ((*lineptr + *n - cur_pos) < 2) {
			if (SSIZE_MAX / 2 < *n) {
				errno = EOVERFLOW;
				return -1;
			}
			new_lineptr_len = *n + BUF_INC;
			new_lineptr = realloc(*lineptr, new_lineptr_len);
			if (new_lineptr == NULL) {
				errno = ENOMEM;
				return -1;
			}
			cur_pos = new_lineptr + (cur_pos - *lineptr);
			*lineptr = new_lineptr;
			*n = new_lineptr_len;
		}
		*cur_pos++ = (char)c;
		if (c == delimiter) {
		    break;
		}
	}

	*cur_pos = '\0';
	return (ssize_t)(cur_pos - *lineptr);
}
#endif /* LIBSOL10_COMPAT_HAVE_GETDELIM */

#ifndef LIBSOL10_COMPAT_HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *restrict fp)
{
	return getdelim(lineptr, n, '\n', fp);
}
#endif /* LIBSOL10_COMPAT_HAVE_GETLINE */

/* open_memstream is used in some newer software */
/* thanks to ChatGPT for help with the code */

#ifndef LIBSOL10_COMPAT_HAVE_OPEN_MEMSTREAM
FILE* open_memstream(char** ptr, size_t* size) {
	if (ptr == NULL || size == NULL) {
		return NULL; /* Invalid arguments */
	}

	memstream_t* ms = malloc(sizeof(memstream_t));
	if (NULL == ms) {
		return NULL; /* Memory allocation failed */
	}

	ms->capacity = 256; /* Initial capacity */
	ms->size = 0;
	ms->buffer = malloc(ms->capacity);
	if (NULL == ms->buffer) {
		free(ms);
		return NULL; /* Memory allocation failed */
	}

	ms->buffer[0] = '\0'; /* Null-terminate the buffer */

	/* Open a temporary file to simulate the stream */
	FILE* stream = tmpfile();
	if (NULL == stream) {
		free(ms->buffer);
		free(ms);
		return NULL; /* Failed to open a temporary file */
	}

	/* Set output pointers */
	*ptr = ms->buffer;
	*size = ms->size;

	return stream;
}
#endif /* LIBSOL10_COMPAT_HAVE_OPEN_MEMSTREAM */
