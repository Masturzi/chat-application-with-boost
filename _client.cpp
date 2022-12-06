#include <iostream>
#include <string>
#include <array>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

class ChatClient
{
public:
  ChatClient(boost::asio::io_context& io_context, const std::string& server, const std::string& port)
    : socket_(io_context)
  {
    // Resolve the address of the server.
    tcp::resolver resolver(io_context);
    boost::asio::connect(socket_, resolver.resolve(server, port));

    do_read_header();
  }

  void write(std::string message)
  {
    boost::property_tree::ptree pt;
    pt.put("sender", "client");
    pt.put("recipient", "server");
    pt.put("text", message);

  std::ostringstream os;
  boost::property_tree::write_json(os, pt);
  std::string message_json = os.str();

  do_write(message_json);
}

private:
  void do_read_header()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(header_, header_length),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                              if (!ec)
                              {
                                do_read_body();
                              }
                            });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(body_, body_length_),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                              if (!ec)
                              {
                                do_handle_message();
                              }
                            });
  }

  void do_handle_message()
  {
    std::string message(body_, body_length_);
    BOOST_LOG_TRIVIAL(info) << "Received message from server: " << message;

    boost::property_tree::ptree pt;
    std::istringstream is(message);
    boost::property_tree::read_json(is, pt);

    std::string sender = pt.get<std::string>("sender");
    std::string text = pt.get<std::string>("text");

    std::cout << sender << ": " << text << std::endl;

    do_read_header();
  }

  void do_write(std::string message)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(message),
                             [this, self](boost::system::error_code ec, std::size_t)
                             {
                               if (!ec)
                               {
                                 do_read_header();
                               }
                             });
  }

  tcp::socket socket_;
  std::array<char, 4> header_;
  uint32_t body_length_ = 0;
  std::array<char, 1024> body_;
};

int main()
{
  try
  {
    // Create the io_context object that will manage all of our network communication.
    boost::asio::io_context io_context;

    // Create a chat client and connect to the server.
    ChatClient client(io_context, "localhost", "1337");

    // Read messages from the user and send them to the server.
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
