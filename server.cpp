/* 
 * File:   sserver.cpp
 * Author: dsultanov
 *
 * Created on January 11, 2017
 */

#include <sserver.h>

namespace server_ns
{

connection::connection(std::string const& ip_addr, int port)
  : connection_base(ip_addr, port)
{
}

void connection::start()
{
  start_connection(socket_ptr(new boost::asio::ip::tcp::socket(ioservice)));
  connection_base::start();
}

void connection::start_connection(socket_ptr s)
{
  if(!s)
    return;
  acc->async_accept(*s, [this, s](const boost::system::error_code & e)
  {
    connection_handler(s, e);
  });
}

void connection::connection_handler(socket_ptr s, const boost::system::error_code & e)
{
  using boost::asio::buffer;
  if(!s || e)
    return;
  int index_ = index;
  s->async_read_some(buffer(bytes), [this, s, index_](boost::system::error_code const& e,
    std::size_t nbytes) {
    read_handler(index_, s, e, nbytes);
  });
  std::cout << index << " connected" << std::endl;
  index++;
  start_connection(socket_ptr(new boost::asio::ip::tcp::socket(ioservice)));
}

void connection::read_handler(int i, socket_ptr s, boost::system::error_code const& e, std::size_t nbytes)
{
  using boost::asio::buffer;
  if(!s || e)
    return;
  std::string str(bytes.data(), bytes.data()+nbytes-1);
  std::cout << i << " " << nbytes << " bytes received: " << str;
  boost::asio::write(*s, buffer("received\n"));
  s->async_read_some(buffer(bytes),
                     [this, s, i](boost::system::error_code const& e,
                     std::size_t nbytes) {
    read_handler(i, s, e, nbytes);
  });
};

} // namespace server_ns