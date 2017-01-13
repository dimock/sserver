/* 
 * File:   sserver.cpp
 * Author: dsultanov
 *
 * Created on January 11, 2017
 */

#include <sserver.h>
#include <numeric>
#include <boost/algorithm/string/join.hpp>

namespace server_ns
{

connection::connection(std::string const& ip_addr, int port, int wite_timeout, std::string const& output_file)
  : connection_base(ip_addr, port),
    wite_timeout_(wite_timeout),
    output_file_(output_file)
{
  using namespace boost::asio::ip;
  acc = std::unique_ptr<tcp::acceptor>{new tcp::acceptor(ioservice, *ep)}; 
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
  logger_ns::logger(logger_ns::message_type::M_INFO, index, "-th client is connected");
  index++;
  start_connection(socket_ptr(new boost::asio::ip::tcp::socket(ioservice)));
}

void connection::read_handler(int i, socket_ptr s, boost::system::error_code const& e, std::size_t nbytes)
{
  using boost::asio::buffer;
  if(!s || e)
    return;  
  process_received_data(i, s, nbytes);
  s->async_read_some(buffer(bytes),
                     [this, s, i](boost::system::error_code const& e,
                     std::size_t nbytes) {
    read_handler(i, s, e, nbytes);
  });
}

void connection::process_received_data(int i, socket_ptr s, std::size_t nbytes)
{
  using boost::asio::buffer;
  size_t nnumbers = nbytes / sizeof(value_type);
  value_type const* numbers = reinterpret_cast<value_type const*>(bytes.data());
  std::vector<std::string> str_numbers;
  std::transform(numbers, numbers + nnumbers,
                 std::back_inserter(str_numbers),
                 [this](value_type v)
                 {
                   // side effect
                   values_[v]++;
                   return std::to_string(v);
                 });
  auto str = boost::algorithm::join(str_numbers, ",");
  logger_ns::logger(logger_ns::message_type::M_INFO, i, "-th client sent ", nbytes, " bytes with numbers set {", str, "}");
  result_type result = calculate_average();
  boost::asio::write(*s, buffer(&result, sizeof(result)));
}

// this version has been written non-optimized to clarify algorithm
// it is possible to store summ and count somewhere but
// requires some tricks to have deal with calculation error
result_type connection::calculate_average()
{
  // calculate weighted average based on quantity of each key  
  std::int64_t count{};
  std::vector<double> squares;
  squares.reserve(values_.size());
  for(auto const& kv : values_)
  {
    squares.push_back(kv.first * kv.first * kv.second);
    count += kv.second;
  }
  if(count == 0)
    return result_type{};
  // perform summation starting from smallest number to eliminate error
  std::sort(squares.begin(), squares.end());
  return std::accumulate(squares.begin(), squares.end(), 0) / count;
}

} // namespace server_ns
