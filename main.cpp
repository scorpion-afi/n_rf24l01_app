/*
 * main.cpp
 *
 *  Created on: Jun 21, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <array>
#include <cstring>

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include "n_rf24l01_library/linux/n_rf24l01_linux.h"

const std::size_t segment_size = 512;
int n_rf_fd;

/* read data, up to @c segment_size bytes, from the file identified by an @c fd fd and return
 * result as a vector<char> */
std::vector<char> read_data( int fd )
{
  std::size_t available_space, current_offset;
  std::vector<char> buff( segment_size );
  int ret;

  available_space = buff.size();
  current_offset = 0;
  
  while( 1 )
  {
    /* try to read up to buff.size() bytes from the input stream */
    ret = read( fd, &buff.front() + current_offset, available_space );

    if( ret < 0 && errno == EINTR )
      continue;

    /* no more data to read */
    if( ret < 0 && errno == EWOULDBLOCK )
      break;

    if( ret < 0 )
    {
      int err = errno;
      std::array<char, 256> error_buf;

      throw std::string( std::string("fail to read from an input stream: ") +
                         strerror_r( err, &error_buf.front(), error_buf.size() ) );
    }

    available_space -= ret;
    current_offset += ret;

    if( available_space == 0 )
      break;
  }

  /* return only useful bytes */
  buff.resize( buff.size() - available_space );

  return buff;
}

/* write, data.size() bytes, @c data data to the file identified by an @c fd fd */
void write_data( int fd, const std::vector<char>& data )
{
  std::size_t count_to_write, current_offset;
  int ret;

  count_to_write = data.size();
  current_offset = 0;

  while( 1 )
  {
    /* try to write up to count_to_write bytes to the device */
    ret = write( fd, &data.front() + current_offset, count_to_write );

    if( ret < 0 && errno == EINTR )
      continue;

    /* keep on till all data is written */
    if( ret < 0 && errno == EWOULDBLOCK )
      continue;

    if( ret < 0 )
    {
      int err = errno;
      std::array<char, 256> error_buf;

      throw std::string( std::string("fail to write to an n_rf24l01 device: ") + strerror_r( err, &error_buf.front(), error_buf.size() ) );
    }

    count_to_write -= ret;
    current_offset += ret;

    /* no more data to write */
    if( count_to_write == 0 )
      break;
  }
}

/* read data, up to @c segment bytes, from an input stream and write them to an n_rf24l01 device */
void some_data_on_input_stream()
{
  std::cout << "read from an input stream...\n";
  std::vector<char> data = read_data( STDIN_FILENO );

  std::cout << "write to the n_rf24l01 device...\n";
  write_data( n_rf_fd, data );

  std::cout << "...ok.\n";
}

/* print out a data from an n_rf24l01 device */
void some_data_on_n_rf24l01_device()
{
  std::cout << "read data from the n_rf24l01 device...\n";
  std::vector<char> data = read_data( n_rf_fd );

  std::cout << " ";
  for( const auto& ch : data )
    std::cout << ch;

  std::cout << std::endl;
}

/* this program forwards data from an input stream to an n_rf24l01 device
 * and prints out data available (to read) on the device. */
int main ( void )
{
  try
  {
    pollfd events_fd[2];

    n_rf_fd = n_rf24l01_open();
    if( n_rf_fd < 0 )
      throw std::string( "fail to open an n_rf24l01 device" );

    events_fd[0].events = POLLIN;
    events_fd[0].fd = STDIN_FILENO;
    events_fd[1].events = POLLIN;
    events_fd[1].fd = n_rf_fd;

    /* make all fds non-blocking */
    for( auto& ev : events_fd )
    {
      int file_status_flags = fcntl( ev.fd, F_GETFL );
      fcntl( ev.fd, F_SETFL, file_status_flags | O_NONBLOCK );
    }

    while( 1 )
    {
      int ret;

      ret = poll( events_fd, sizeof(events_fd) / sizeof(pollfd), -1 );
      if( ret < 0 && errno == EINTR )
        continue;

      if( ret < 0 )
        throw std::string( "an error while a poll syscall." );

      if( events_fd[0].revents == POLLIN )
        some_data_on_input_stream();

      if( events_fd[1].revents == POLLIN )
        some_data_on_n_rf24l01_device();
    }
  }
  catch( const std::string& except )
  {
    std::cout << "an exception: " << except << std::endl;
  }
  catch( const std::exception& except )
  {
    std::cout << "an std exception: " << except.what() << std::endl;
  }

  return 0;
}
