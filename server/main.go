package main

import (
    "fmt"
    "net"
    "os"
)

const (
    CONN_HOST = "localhost"
    CONN_PORT = "8001"
    CONN_TYPE = "tcp"

	CONN_MAX_SIZE = 8191
)

func main() {

    // Listen for incoming connections
    l, err := net.Listen(CONN_TYPE, CONN_HOST + ":" + CONN_PORT)
    if err != nil {
        fmt.Printf("Error listening on %s:%s: %s", CONN_HOST, CONN_PORT,
			err.Error())
        os.Exit(1)
    }

    defer l.Close()

    fmt.Println("Listening on " + CONN_HOST + ":" + CONN_PORT)
    for {
		// Accept connection
        conn, err := l.Accept()
        if err != nil {
            fmt.Println("Error accepting connection: ", err.Error())
            os.Exit(1)
        }
		// Launch goroutine to service connection
        go handleRequest(conn)
    }
}

func handleRequest(conn net.Conn) {
	buf := make([]byte, CONN_MAX_SIZE)

	readLen, err := conn.Read(buf)
	if err != nil {
		fmt.Println("Error reading from client:", err.Error())
	}
	fmt.Printf("Got %d bytes from client\n", readLen);
	fmt.Printf("rx: % X\n", buf[:readLen])
	//conn.Write([]byte("Message received"))
	conn.Close()
}
