#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main()
{
  try
  {
    // Create the io_context object that will manage all of our network communication.
    boost::asio::io_context io_context;

    // Create a tcp::resolver object and use it to look up the address of the server we want to connect to.
    tcp::resolver resolver(io_context);
    auto endpoint = resolver.resolve("localhost", "1337");

    // Create a tcp::socket object and use it to connect to the server.
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoint);

    // Create a std::string to hold the message we want to send to the server.
    std::string message = "Hello, server!";

    // Use the boost::asio::write function to send the message to the server.
    boost::asio::write(socket, boost::asio::buffer(message));

    // Create a buffer to hold the response from the server.
    std::array<char, 128> 
    // Create a buffer to hold the response from the server.
    std::array<char, 128> buffer;

    // Use the boost::asio::read function to read the response from the server into the buffer.
    size_t bytes_transferred = boost::asio::read(socket, boost::asio::buffer(buffer));

    // Print out the response from the server.
    std::cout << "Response from server: ";
    std::cout.write(buffer.data(), bytes_transferred);
    std::cout << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

