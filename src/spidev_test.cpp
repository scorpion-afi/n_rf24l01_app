/*
 ============================================================================
 Name        : spidev_test.cpp
 Author      : sergs
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/eventfd.h>


/* am ioctl request to pass event_fd object's fd to the n_rf24l01 device driver
 *
 * 248 - is a major device number allocated by the kernel for the n_rf24l01 device driver
 * 1   - is a number of the ioctl
 * int - is a type of the parameter */
#define PASS_EVENT_FD _IOW( 248, 1, int )

const std::string n_rf24l01_device_name = "/dev/n_rf24l01";


int main ( void )
{
  int event_fd;
  uint64_t is_event;
  int res;

  int n_rf24l01_dev_fd;

  n_rf24l01_dev_fd = open( n_rf24l01_device_name.c_str(), O_RDWR | O_CLOEXEC );
  if( n_rf24l01_dev_fd < 0 )
  {
    perror( "fail to call open n_rf24l01 device file" );
    return 1;
  }

  /* create an event_fd object */
  event_fd = eventfd( 0, EFD_CLOEXEC );
  if( event_fd < 0 )
  {
    perror( "fail to create event_fd object" );
    close( n_rf24l01_dev_fd );

    return 1;
  }

  /* pass the event_fd object's fd to the n_rf24l01 device driver */
  res = ioctl( n_rf24l01_dev_fd, PASS_EVENT_FD, event_fd );
  if( res < 0 )
  {
      perror( "fail to pass event_fd object's fd to the kernel driver" );
      close( event_fd );
      close( n_rf24l01_dev_fd );

      return 1;
  }

  std::cout << "getting waited for an event from kernel..." << std::flush;

  /* as an eventfd object has been created with an inner counter being zero-initialized this
   * read operation causes thread to be blocked till inner counter gets some value (in our
   * case it's the kernel driver responsibility to do this)
   * Note: after read has been completed the inner counter will be reset to zero
   *  (EFD_SEMAPHORE was not specified) */
  res = read( event_fd, &is_event, sizeof(is_event) );
  if( res < 0 )
  {
    perror( "fail to call read syscall" );
    close( event_fd );
    close( n_rf24l01_dev_fd );

    return 1;
  }

  std::cout << " success (is_event:" << is_event << "). Try to add '0xfffffffffffffffe' to the inner counter..."
      << std::flush;

  /* to cause an write operation blocks we write the largest value we can write anyway */
  is_event = 0xfffffffffffffffe;

  /* this call shouldn't block... */
  res = write( event_fd, &is_event, sizeof(is_event) );
  if( res < 0 )
  {
    perror( "fail to call write syscall" );
    close( event_fd );
    close( n_rf24l01_dev_fd );

    return 1;
  }

  std::cout << " success. Try to add '1' to the inner counter..."
      << std::flush;

  is_event = 1;

  /* but this one should block till somebody, in our case the kernel driver, decreases inner counter to allow
   * this write operation being completed */
  res = write( event_fd, &is_event, sizeof(is_event) );
  if( res < 0 )
  {
    perror( "fail to call write syscall" );
    close( event_fd );
    close( n_rf24l01_dev_fd );

    return 1;
  }

  std::cout << " success. Somebody decreased inner counter value.\n";

  close( event_fd );
  close( n_rf24l01_dev_fd );
}
