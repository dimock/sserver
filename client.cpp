/*
* File:   sserver.cpp
* Author: dsultanov
*
* Created on January 11, 2017
*/

#include <sserver.h>

namespace client_ns
{

connection::connection(std::string const& ip_addr, int port, int send_timeout)
  : connection_base(ip_addr, port),
    send_timeout_(send_timeout),
    generator{ rdevice() },
    distribution{ 0, 1023 }
{
  using namespace boost::asio;
  using namespace boost::asio::ip;

  sock = socket_ptr(new tcp::socket{ ioservice });
  sock->async_connect(*ep, [&, this](boost::system::error_code const& e)
  {
    if(!sock ||e) {
      return;
    }
    send_random_number();    
    sock->async_read_some(buffer(bytes),
                           [this](boost::system::error_code const& e, std::size_t nbytes)
    {
      read_handler(e, nbytes);
    });
  });
}

void connection::read_handler(boost::system::error_code const& e, std::size_t nbytes)
{
  using namespace boost::asio;
  using namespace boost::asio::ip;
  if(!sock || e)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, " error received ", e.message());
    return;
  }
  print_received_data(nbytes);
  std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(send_timeout_));
  if(!send_random_number())
    return;
  sock->async_read_some(buffer(bytes),
                        [this](boost::system::error_code const& e, std::size_t nbytes)
  {
    read_handler(e, nbytes);
  });
}

bool connection::send_random_number()
{
  using namespace boost::asio;
  try
  {
    value_type number = distribution(generator);
    write(*sock, buffer(&number, sizeof(number)));
  }
  catch(std::exception& e)
  {
    logger_ns::logger(logger_ns::message_type::M_ERROR, " exception ", e.what());
    return false;
  }
  return true;
}

void connection::print_received_data(size_t nbytes)
{
  size_t nnumbers = nbytes / sizeof(result_type);
  result_type const* numbers = reinterpret_cast<result_type const*>(bytes.data());
  for(size_t i = 0; i < nnumbers; ++i, ++numbers)
  {
    auto number = *numbers;
    logger_ns::logger(logger_ns::message_type::M_INFO, " received value = ", number);
  }
}

} // namespace client_ns
