#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <array>
#include <memory>
#include <utility>

#include <cstring>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>

#include "n_rf24l01_library/linux/n_rf24l01_linux.h"

/* This simple app uses a libn_rf24l01 library to get an access to the n_rf24l01's registers map,
 * it's can be considered like a test utility to test the libn_rf24l01 library and/or
 * the n_rf24l01 transceiver itself */

/* TODO: C++11 provides a regular expression support, switch to it, 'cause the current implementation
 *       looks like a garbage-code... */

/* these function prototypes ain't exposed explicitly by the n_rf24l01 library,
 * so they don't exist within its header file */
extern "C" {
uint64_t n_rf24l01_read_register_dbg( u_char reg_addr );
void n_rf24l01_write_register_dbg( u_char reg_addr, uint64_t value );
}

/* map orders elements not in the order I wish them be */
const std::vector<std::pair<std::string, char>> registers_name_map =
{
  { "CONFIG_RG",      0x00 },
  { "EN_AA_RG",       0x01 },
  { "EN_RXADDR_RG",   0x02 },
  { "SETUP_AW_RG",    0x03 },
  { "SETUP_RETR_RG",  0x04 },
  { "RF_CH_RG",       0x05 },
  { "RF_SETUP_RG",    0x06 },
  { "STATUS_RG",      0x07 },
  { "OBSERVE_TX_RG",  0x08 },
  { "RPD_RG",         0x09 },
  { "RX_ADDR_P0_RG",  0x0a },
  { "RX_ADDR_P1_RG",  0x0b },
  { "RX_ADDR_P2_RG",  0x0c },
  { "RX_ADDR_P3_RG",  0x0d },
  { "RX_ADDR_P4_RG",  0x0e },
  { "RX_ADDR_P5_RG",  0x0f },
  { "TX_ADDR_RG",     0x10 },
  { "RX_PW_P0",       0x11 },
  { "RX_PW_P1",       0x12 },
  { "RX_PW_P2",       0x13 },
  { "RX_PW_P3",       0x14 },
  { "RX_PW_P4",       0x15 },
  { "RX_PW_P5",       0x16 },
  { "FIFO_STATUS",    0x17 },
  { "DYNPD",          0x1c },
  { "FEATURE",        0x1d },
};

static std::string _usage_example();

/* describes a general command to interpret */
class command
{
public:
  explicit command( std::string cmd_name ) : cmd_name(std::move(cmd_name)) {}
  virtual ~command() = default;

  command( const command& rhs ) = default;
  command& operator=( const command& rhs ) = default;

  command( command&& rhs ) = default;
  command& operator=( command&& rhs ) = default;

  const std::string& get_cmd_name() const { return cmd_name; }

  /* has to be called if a @cmd_name substr exists in a cml_string */
  virtual void operator()( const std::string& cml_string ) = 0;

private:
  std::string cmd_name;
};

class write_cmd : public command
{
public:
  explicit write_cmd( std::string cmd_name ) : command( std::move(cmd_name) ) {}

  void operator()( const std::string& cml_string ) override
  {
    std::string::size_type pos;

    for( const auto& reg : registers_name_map )
    {
      pos = cml_string.find( reg.first );
      if( pos != std::string::npos )
      {
        uint64_t value;

        pos += reg.first.size() + sizeof(" ");  /* advance a pos to the value */

        value = std::strtoull( cml_string.data() + pos, nullptr, 0 );
        n_rf24l01_write_register_dbg( reg.second, value );

        return;
      }
    }

    throw std::string( "an invalid command's argument(s)" + _usage_example() );
  }
};

class read_cmd : public command
{
public:
  explicit read_cmd( std::string cmd_name ) : command( std::move(cmd_name) ) {}

  void operator()( const std::string& cml_string ) override
  {
    /* read all registers */
    if( cml_string.find( "all" ) != std::string::npos )
    {
      for( const auto& reg : registers_name_map )
        std::cout << reg.first << ": 0x" << std::hex << n_rf24l01_read_register_dbg( reg.second ) << std::endl;

      return;
    }

    for( const auto& reg : registers_name_map )
    {
      if( cml_string.find( reg.first ) != std::string::npos )
      {
        std::cout << reg.first << ": 0x" << std::hex << n_rf24l01_read_register_dbg( reg.second ) << std::endl;
        return;
      }
    }

    throw std::string( "an invalid command's argument(s)" + _usage_example() );
  }
};

/* a vector's ctor copies elements from an initializer list,
 * so we can't use unique_ptr with such a type of a vector initialization... */
const std::vector<std::shared_ptr<command>> cmds_set =
{
  std::make_shared<read_cmd>( std::string("read") ),
  std::make_shared<write_cmd>( std::string("write") ),
};

std::string _usage_example()
{
  std::stringstream s_stream;

  s_stream << "\n\nusage:\n";
  s_stream << " <command> <data>\n";
  s_stream << " available @commands:\n";

  for( const auto& cmd : cmds_set )
    s_stream << "  " << cmd->get_cmd_name() << std::endl;

  s_stream << " @data can be a register name,\n";
  s_stream << " available register names:\n";

  for( const auto& reg : registers_name_map )
    s_stream << "  " << reg.first << std::endl;

  s_stream << " digital, as a @data, can be typed in different number systems";

  return s_stream.str();
}

static void _process_input( const std::string& cml_string )
{
  for( auto& cmd : cmds_set )
  {
    /* some hack to make a user to type a ' ' ch between command and data */
    if( cml_string.find( cmd->get_cmd_name() + " " ) != std::string::npos )
    {
      (*cmd)( cml_string );
      return;
    }
  }

  throw std::string( "an invalid command" + _usage_example() );
}

const std::size_t segment_size = 512;

/* read data, up to @c segment_size bytes, from the file identified by an @c fd fd and return
 * result as a std::string */
static std::string _read_data( int fd )
{
  std::size_t available_space, current_offset;
  std::string buff( segment_size, 'A' );  /* Why 'A', why not? */
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

      throw std::string( std::string("fail to read from a file: ") +
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

static void _some_data_on_input_stream()
{
  std::string cml_string = _read_data( STDIN_FILENO );

  _process_input( cml_string );
}

int main( void )
{
  try
  {
    pollfd events[1];

    if( n_rf24l01_open_dbg() < 0 )
      throw std::string( "fail to open an n_rf24l01 device in the debug mode" );

    events[0].events = POLLIN;
    events[0].fd = STDIN_FILENO;

    /* make all fds non-blocking */
    for( auto& ev : events )
    {
      int file_status_flags = fcntl( ev.fd, F_GETFL );
      fcntl( ev.fd, F_SETFL, file_status_flags | O_NONBLOCK );
    }

    while( 1 )
    {
      int ret;

      ret = poll( events, sizeof(events) / sizeof(pollfd), -1 );
      if( ret < 0 && errno == EINTR )
        continue;

      if( ret < 0 )
        throw std::string( "an error while a poll syscall." );

      if( events[0].revents == POLLIN )
        _some_data_on_input_stream();
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
}
