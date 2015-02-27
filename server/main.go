package main

import (
	"bufio"
    "fmt"
	"io"
    "net"
    "os"
)

const (
    CONN_HOST = "localhost"
    CONN_PORT = "8001"
    CONN_TYPE = "tcp"

	CONN_MAX_SIZE = 8191
)

type Client struct {
	connection *net.Conn
	incoming chan []byte
	outgoing chan []byte
	reader   *bufio.Reader
	writer   *bufio.Writer
	clients  *map[string]*Client
}

func (client *Client) Read() {
	data := make([]byte, CONN_MAX_SIZE)
	for {
		n, err := client.reader.Read(data)
		if (err != nil) {
			if (err == io.EOF) {
				fmt.Printf("Client disconnected\n")
			}
			fmt.Printf("Error reading from client\n")
			continue
		}
		client.incoming <- data[:n]
	}
}

func (client *Client) Write() {
	for data := range client.outgoing {
		_, err := client.writer.Write(data)
		client.writer.Flush()
		if (err != nil) {
			fmt.Printf("Error writing to client\n")
			continue
		}
	}
}

func (client *Client) Listen() {
	go client.Read()
	go client.Write()
}

func NewClient(connection net.Conn, clients *map[string]*Client) *Client {
	client := &Client{
		connection: &connection,
		incoming: make(chan []byte, CONN_MAX_SIZE),
		outgoing: make(chan []byte, CONN_MAX_SIZE),
		reader:   bufio.NewReader(connection),
		writer:   bufio.NewWriter(connection),
		clients:  clients,
	}
	client.Listen()
	return client
}

type Server struct {
	clients  map[string]*Client
	joins    chan net.Conn
	incoming chan []byte
	outgoing chan []byte
}

func (server *Server) Broadcast(data []byte) {
	for _, client := range server.clients {
		client.outgoing <- data
	}
}

func (server *Server) Join(connection net.Conn) {
	client := NewClient(connection, &server.clients)
	server.clients[connection.RemoteAddr().String()] = client
	go func() {
		for {
			server.incoming <- <-client.incoming
		}
	}()
}

func (server *Server) Listen() {
	go func() {
		for {
			select {
			case data := <-server.incoming:
				server.Broadcast(data)
			case conn := <-server.joins:
				server.Join(conn)
			}
		}
	}()
}

func NewServer() *Server {
	server := &Server{
		clients:  make(map[string]*Client),
		joins:    make(chan net.Conn),
		incoming: make(chan []byte, CONN_MAX_SIZE),
		outgoing: make(chan []byte, CONN_MAX_SIZE),
	}
	server.Listen()
	return server
}

func main() {
	server := NewServer()
	l, err := net.Listen(CONN_TYPE, CONN_HOST + ":" + CONN_PORT)
    if err != nil {
        fmt.Printf("Error listening on %s:%s: %s", CONN_HOST, CONN_PORT,
			err.Error())
        os.Exit(1)
    }

	defer l.Close()

    fmt.Println("Listening on " + CONN_HOST + ":" + CONN_PORT)
	for {
		conn, err := l.Accept()
        if err != nil {
            fmt.Println("Error accepting connection: ", err.Error())
            os.Exit(1)
        }
		server.joins <-conn
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
