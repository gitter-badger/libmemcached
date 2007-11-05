/*
  Basic socket buffered IO
*/

#include "common.h"
#include "memcached_io.h"
#include <sys/select.h>
#include <poll.h>

static int io_wait(memcached_st *ptr, unsigned int server_key, unsigned read_or_write)
{
  struct pollfd fds[1];
  short flags= 0;

  if (read_or_write)
    flags= POLLOUT |  POLLERR;
  else
    flags= POLLIN | POLLERR;

  memset(&fds, 0, sizeof(struct pollfd));
  fds[0].fd= ptr->hosts[server_key].fd;
  fds[0].events= flags;

  if (poll(fds, 1, -1) < 0)
    return MEMCACHED_FAILURE;

  return MEMCACHED_SUCCESS;
}

ssize_t memcached_io_read(memcached_st *ptr, unsigned  int server_key,
                          char *buffer, size_t length)
{
  char *buffer_ptr;

  buffer_ptr= buffer;

  while (length)
  {
    if (!ptr->read_buffer_length)
    {
      size_t data_read;

      while (1)
      {
        if (ptr->flags & MEM_NO_BLOCK)
        {
          memcached_return rc;

          rc= io_wait(ptr, server_key, 0);
          if (rc != MEMCACHED_SUCCESS)
            return -1;
        }

        data_read= read(ptr->hosts[server_key].fd, 
                        ptr->read_buffer, 
                        MEMCACHED_MAX_BUFFER);
        if (data_read == -1)
        {
          switch (errno)
          {
          case EAGAIN:
            break;
          default:
            {
              ptr->cached_errno= errno;
              return -1;
            }
          }
        }
        else if (data_read)
          break;
        /* If zero, just keep looping */
      }

      ptr->read_buffer_length= data_read;
      ptr->read_ptr= ptr->read_buffer;
    }

    *buffer_ptr= *ptr->read_ptr;
    length--;
    ptr->read_ptr++;
    ptr->read_buffer_length--;
    buffer_ptr++;
  }

  return (size_t)(buffer_ptr - buffer);
}

ssize_t memcached_io_write(memcached_st *ptr, unsigned int server_key,
                        char *buffer, size_t length, char with_flush)
{
  unsigned long long x;

  for (x= 0; x < length; x++)
  {
    ptr->write_buffer[ptr->write_buffer_offset]= buffer[x];
    ptr->write_buffer_offset++;

    if (ptr->write_buffer_offset == MEMCACHED_MAX_BUFFER)
    {
      size_t sent_length;

      sent_length= memcached_io_flush(ptr, server_key);

      WATCHPOINT_ASSERT(sent_length == MEMCACHED_MAX_BUFFER);
      ptr->write_buffer_offset= 0;
    }
  }

  if (with_flush)
  {
    if (memcached_io_flush(ptr, server_key) == -1)
      return -1;
  }

  return length;
}

ssize_t memcached_io_flush(memcached_st *ptr, unsigned int server_key)
{
  size_t sent_length;
  size_t return_length;
  char *write_ptr= ptr->write_buffer;
  size_t write_length= ptr->write_buffer_offset;
  unsigned int loop= 1;

  if (ptr->write_buffer_offset == 0)
    return 0;

  return_length= 0;
  while (write_length)
  {
    if (ptr->flags & MEM_NO_BLOCK)
    {
      memcached_return rc;

      rc= io_wait(ptr, server_key, 1);
      if (rc != MEMCACHED_SUCCESS)
        return -1;
    }

    sent_length= 0;
    if ((ssize_t)(sent_length= write(ptr->hosts[server_key].fd, write_ptr, 
                                     write_length)) == -1)
    {
      switch (errno)
      {
      case ENOBUFS:
      case EAGAIN:
        WATCHPOINT;
        continue;
        if (loop < 100)
        {
          loop++;
          break;
        }
        /* Yes, we want to fall through */
      default:
        ptr->cached_errno= errno;
        return -1;
      }
    }

    write_ptr+= sent_length;
    write_length-= sent_length;
    return_length+= sent_length;
  }

  WATCHPOINT_ASSERT(write_length == 0);
  WATCHPOINT_ASSERT(return_length == ptr->write_buffer_offset);
  ptr->write_buffer_offset= 0;

  return return_length;
}

/* 
  Eventually we will just kill off the server with the problem.
*/
void memcached_io_reset(memcached_st *ptr, unsigned int server_key)
{
  ptr->write_buffer_offset= 0;
  memcached_quit(ptr);
}