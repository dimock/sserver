/*
* File:   sserver.cpp
* Author: dsultanov
*
* Created on January 11, 2017
*/

#include <sserver.h>

namespace client_ns
{

connection::connection(std::string const& ip_addr, int port)
  : connection_base(ip_addr, port)
{
  using namespace boost::asio;
  using namespace boost::asio::ip;

  sock = socket_ptr(new tcp::socket{ ioservice });
  sock->async_connect(*ep, [&, this](boost::system::error_code const& e)
  {
    if(!sock ||e) {
      return;
    }
    write(*sock, buffer("send something\n"));
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
    return;
  std::string str(bytes.data(), bytes.data()+nbytes-1);
  std::cout << str;
  std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(300));
  write(*sock, buffer("send something\n"));
  sock->async_read_some(buffer(bytes),
                        [this](boost::system::error_code const& e, std::size_t nbytes)
  {
    read_handler(e, nbytes);
  });
};

} // namespace client_ns
