

typedef struct {
  GslDataHandle dhandle;

  GslLong pcm_pos;
  guint	  pcm_length;
  guint   pcm_size;
} LBufferHandle;

/* ring buffer, caching the last n values read */
