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

#ifndef LIBSOL10_COMPAT_HAVE_OPEN_MEMSTREAM
FILE* open_memstream(char** ptr, size_t* size) {
	/* Initialize the memory stream structure */
	memstream_t* ms = malloc(sizeof(memstream_t));
	if (!ms) {
		return NULL; /* Memory allocation failed */
	}

	ms->capacity = 256; /* Initial capacity */
	ms->size = 0;
	ms->buffer = malloc(ms->capacity);
	if (!ms->buffer) {
		free(ms);
		return NULL; /* Memory allocation failed */
	}

	ms->buffer[0] = '\0'; /* Null-terminate the buffer */

	/* Open a temporary file */
	FILE* stream = tmpfile();
	if (!stream) {
		free(ms->buffer);
		free(ms);
		return NULL; /* Failed to open a temporary file */
	}

	/* Custom write function to handle dynamic buffer growth */
	size_t write_to_memstream(const void* data, size_t size, size_t nmemb, FILE* file) {
		size_t total_size = size * nmemb;
		if (ms->size + total_size >= ms->capacity) {
			size_t new_capacity = ms->capacity * 2;
			while (ms->size + total_size >= new_capacity) {
				new_capacity *= 2;
			}
			char* new_buffer = realloc(ms->buffer, new_capacity);
			if (!new_buffer) {
				return 0; /* Memory allocation failed */
			}
			ms->buffer = new_buffer;
			ms->capacity = new_capacity;
		}

		// Copy data to the buffer
		memcpy(ms->buffer + ms->size, data, total_size);
		ms->size += total_size;
		ms->buffer[ms->size] = '\0'; /* Null-terminate */
		return nmemb;
	}

	/* Link the buffer and size to the caller's pointers */
	*ptr = ms->buffer;
	*size = ms->size;

	/* Replace the file's default write handler with our custom one */
	setbuf(stream, ms->buffer); /* Attach the buffer to the stream */

	return stream;
}
#endif /* LIBSOL10_COMPAT_HAVE_OPEN_MEMSTREAM */
