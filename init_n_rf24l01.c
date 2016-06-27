// This file contains functions to work with n_rf24l01 transceiver via /dev/spidevA.B device file (for odroid U3)

// n_rf24l01 commands have 8 bits.
// Every new command must be started by a high to low transition on CSN.
// In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on
// the MISO pin.

// The serial shifting SPI commands is in the following format:
//  <Command word: MSBit to LSBit (one byte)>
//  <Data bytes: LSByte to MSByte, MSBit in each byte first>

// spi on n_rf24l01 works with next settings:
//  CPOL = 0, CPHA = 0
//  msbit first
//  8 bits per word
//  spi speed up to 8MHz (but now we use only 50kHz)
//  CSN - is active low

#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "n_rf24l01_port.h"

#include "init_n_rf24l01.h"

#define SPI_DEVICE_FILE "/dev/spidev1.0"

// defines for CE pin (n_rf24l01 specified pin)
#define CE_EXPORT_NAME   "199" // J4(IO-Port#1), 1-st pin
#define CE_PIN_FILE      "/sys/class/gpio/gpio"CE_EXPORT_NAME"/"
#define CE_PIN_DIRECTION "out"

// spidev device file fd
static int spi_fd;

// ce pin control file fd
static int ce_pin_fd;


/**
 * @brief control CE pin state
 *
 * void (*set_up_ce_pin_ptr)( u_char value );
 *
 * @param[in] value - new state of CE pin (0 - '0' logical level, 1 - '1' logical level)
 */
//======================================================================================================
static void set_up_ce_pin( u_char value )
{
	int ret;
	char str[2];	// snprintf appends string by '\0' symbol

	// write or 1 or 0
	snprintf( str, sizeof str, "%u", !!value );

	// pin_fd file requires only '1' or '0', no 1 or 0
	ret = write( ce_pin_fd, str, 1 );
	if( ret < 0 || ret != 1 )
		printf( "error while set_up_pin call.\n" );

	return;
}

/**
 * @brief send command to n_rf24l01 over SPI peripheral
 *
 * void (*send_cmd_ptr)( u_char cmd, u_char* status_reg, u_char* data, u_char num, u_char direction );
 *
 * @param[in]     cmd        - command to send
 * @param[out]    status_reg - pointer n_rf24l01 status register will be written to
 * @param[in,out] data       - pointer to data to be written to or to be read from n_rf24l01, depends on @type,
 *                             real amount of data to read from/write to is depends on @num parameter
 * @param[in]     num        - amount of bytes to read or write (except command byte)
 * @param[in]     direction  - type of operation: 1 - write, 0 - read
 * @return -1 if failed, 0 otherwise
 *
 * Note: this function must implements sending command and sending/receiving command's data over spi.
 *       First you must send @cmd, retrieve answer and store it to @status_reg, and then send @num bytes from @data,
 *       or receive @num bytes and store to @data depend of @direction value.
 *       @status_reg may be NULL - just throw respond of first spi transaction (@cmd).
 *       @num may be 0 - in this case just send @cmd and store respond to @status_reg, if @status_reg isn't NULL.
 *       Memory allocation occurred on calling side.
 */
//======================================================================================================
static void send_cmd( u_char cmd, u_char* status_reg, u_char* data, u_char num, u_char direction )
{
	struct spi_ioc_transfer transfers[2];
	u_char nop = 0xff;
	int ret;

    memset( transfers, 0, sizeof(transfers) );

	// transaction to send command
	transfers[0].tx_buf = &cmd;
	transfers[0].rx_buf = status_reg;
	transfers[0].len = 1;

	// set of transaction to write/read command's data
    if( direction ) // if client want to write some data
    {
        transfers[1].tx_buf = data;
        transfers[1].rx_buf = &nop;
    }
    else	// if client want to read some data
    {
        transfers[1].tx_buf = &nop;
        transfers[1].rx_buf = data;
    }

    transfers[1].len = num;

    // ask to do actually spi fullduplex transactions
    if( !num )
      ret = ioctl( spi_fd, SPI_IOC_MESSAGE(1), transfers );
    else
      ret = ioctl( spi_fd, SPI_IOC_MESSAGE(2), transfers );

    if( ret < 0 )
        perror( "error while SPI_IOC_MESSAGE ioctl" );
    else
      printf( "number of transmitted bytes in each spi transactions: %d.\n", ret );
}

/**
 * @brief put to sleep library execution flow, max sleep interval ~1500 ms
 *
 * void (*usleep_ptr)( u_int delay_mks );
 *
 * @param[in] delay_mks - time in microseconds to sleep
 *
 * Note: you can on you own decide how to put to sleep library execution flow,
 *       for example: short sleep interval - just 'for/while' loop,
 *                    long - something else.
 *       library execution flow mustn't obtain control due @delay_mks :-) K.O.
 */
//======================================================================================================
static void _usleep( u_int delay_mks )
{
  usleep( delay_mks );
}

/**
 * @brief handle received data
 *
 * void (*handle_received_data_ptr)( void* data, u_int num );
 *
 * @param[in] data - received from n_rf24l01
 * @param[in] num  - amount of received data, in bytes
 *
 */
//======================================================================================================
void handle_received_data( void* data, u_int num )
{
  printf( "received data: %s\n", (char*)data );
}

// setup odroid's pins needed to control n_rf24l01
// to explain watch to http://forum.odroid.com/viewtopic.php?f=80&t=5702
// return -1 if failed
//======================================================================================================
static int init_pins( void )
{
	struct stat st;
	int fd;
	int ret;

	// check does file CE_PIN_FILE exist, if not, we must create it...
	ret = stat( CE_PIN_FILE, &st );
	if( ret == -1 )
	{
		// create new directory /sys/class/gpio/gpio199,
		// now I don't know how to explain this otherwise
		fd = open( "/sys/class/gpio/export", O_WRONLY );
		if( fd < 0 ) return -1;

		ret = write( fd, CE_EXPORT_NAME, sizeof(CE_EXPORT_NAME) - 1 );
		if( ret < 0 || ret != sizeof(CE_EXPORT_NAME) - 1 )
		{
			printf( "error while write to /sys/class/gpio/export.\n" );
			close( fd );
			return -1;
		}
		close( fd );
	}

	// set ce pin direction
	fd = open( CE_PIN_FILE"direction", O_RDWR );
	if( fd < 0 ) return -1;

	// -1 due to \0 symbol at end of string
	ret = write( fd, CE_PIN_DIRECTION, sizeof(CE_PIN_DIRECTION) - 1 );
	if( ret < 0 || ret != sizeof(CE_PIN_DIRECTION) - 1 )
	{
		printf( "error while set ce pin direction.\n" );
		close( fd );
		return -1;
	}

	close( fd );

	// open file that control ce pin state
	ce_pin_fd = open( CE_PIN_FILE"value", O_RDWR );
	if( ce_pin_fd < 0 ) return -1;

	return 0;
}

// set up spi master to correct settings
// spi on n_rf24l01 works with next settings:
//  CPOL = 0, CPHA = 0
//  msbit first
//  8 bits per word
//  spi speed up to 8MHz (but now we use only 50kHz)
//======================================================================================================
static int setup_master_spi( void )
{
	int ret;
	__u8 mode;
	__u8 bits_order;
	__u8 bits_per_word;
	__u32 speed_hz;

	mode = SPI_MODE_0; // CPOL = 0, CPHA = 0
	ret = ioctl( spi_fd, SPI_IOC_WR_MODE, &mode );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_WR_MODE ioctl call" );
		return -1;
	}

	ret = ioctl( spi_fd, SPI_IOC_RD_MODE, &mode );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_RD_MODE ioctl call" );
		return -1;
	}

	bits_order = 0; // msbit first
	ret = ioctl( spi_fd, SPI_IOC_WR_LSB_FIRST, &bits_order );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_WR_LSB_FIRST ioctl call" );
		return -1;
	}

	ret = ioctl( spi_fd, SPI_IOC_RD_LSB_FIRST, &bits_order );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_RD_LSB_FIRST ioctl call" );
		return -1;
	}

	bits_per_word = 0; // 8 bit per word
	ret = ioctl( spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_WR_BITS_PER_WORD ioctl call" );
		return -1;
	}

	ret = ioctl( spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_RD_BITS_PER_WORD ioctl call" );
		return -1;
	}

	speed_hz = 500000; // 500 kHz
	ret = ioctl( spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_WR_MAX_SPEED_HZ ioctl call" );
		return -1;
	}

	ret = ioctl( spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed_hz );
	if( ret < 0 )
	{
		perror( "error while SPI_IOC_RD_MAX_SPEED_HZ ioctl call" );
		return -1;
	}

	printf( "spi mode: SPI_MODE_%hhu.\n", mode );
	printf( "spi bits order: %s.\n", bits_order ? "lsbit first" : "msbit first" );
	if (!bits_per_word)
		printf( "spi bits per word: 8.\n" );
	else
		printf( "spi bits per word: %hhu.\n", bits_per_word );
	printf( "spi speed: %uHz.\n\n", speed_hz );

	return 0;
}

/**
 * @brief transmit string to space
 *
 * @param[in] data - pointer to null-terminated string
 */
//======================================================================================================
void transmit_str( char* data )
{
  if( !data )
    return;

  n_rf24l01_transmit_pkgs( data, strlen(data) + 1 );
}

//  make all initialization step to prepare n_rf24l01 to work
//======================================================================================================
int init_n_rf24l01( void )
{
  int ret;
  n_rf24l01_backend_t backend;

  spi_fd = open( SPI_DEVICE_FILE, O_RDWR );
  if( spi_fd < 0 )
  {
      char temp[128];

      snprintf( temp, sizeof temp, "error while open spidev device file: %s", SPI_DEVICE_FILE );
      perror( temp );

      return -1;
  }

  ret = setup_master_spi();
  if( ret < 0 )
  {
      close( spi_fd );
      return -1;
  }

  printf( "spi master was successfully prepared to use.\n" );

  ret = init_pins();
  if( ret < 0 )
  {
      close( spi_fd );
      return -1;
  }

  printf( "pins were successfully prepared to use.\n" );

  backend.set_up_ce_pin = set_up_ce_pin;
  backend.send_cmd = send_cmd;
  backend.usleep = _usleep;
  backend.handle_received_data = handle_received_data;

  n_rf24l01_init( &backend );
  n_rf24l01_prepare_to_transmit();

  printf( "n_rf24l01 was successfully prepared to use.\n\n" );

  return 0;
}
