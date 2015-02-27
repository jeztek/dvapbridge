package main

import (
	"bufio"
    "fmt"
	"io"
    "net"
    "os"
)

// Server parameters
const (
    CONN_HOST = "localhost"
    CONN_PORT = "8001"
    CONN_TYPE = "tcp"

	CONN_MAX_SIZE = 8191
)

// Control message
type MsgType int
const (
	MsgDisconnect MsgType = iota
	MsgData       MsgType = iota
)
type Message struct {
	msgtype MsgType
	sender  string
	data    []byte
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

type Server struct {
	clients  map[string]*Client
	joins    chan net.Conn
	control  chan Message
	incoming chan Message
	outgoing chan Message
}

func (server *Server) PrintClients() {
	fmt.Println("Clients:")
	for k := range server.clients {
		fmt.Println(k)
	}
}

func (server *Server) Broadcast(msg Message) {
	for _, client := range server.clients {
		if msg.sender != client.id {
			client.outgoing <- msg
		}
	}
}

func (server *Server) Join(connection net.Conn) {
	client := NewClient(connection, server)
	server.clients[connection.RemoteAddr().String()] = client
	server.PrintClients()
	go func() {
		for {
			server.incoming <- <-client.incoming
		}
	}()
}

func (server *Server) Control(msg Message) {
	if msg.msgtype == MsgDisconnect {
		delete(server.clients, msg.sender)
		server.PrintClients()
	}
}

func (server *Server) Listen() {
	go func() {
		for {
			select {
			case data := <-server.incoming:
				server.Broadcast(data)
			case conn := <-server.joins:
				server.Join(conn)
			case msg := <-server.control:
				server.Control(msg)
			}
		}
	}()
}

func NewServer() *Server {
	server := &Server{
		clients:  make(map[string]*Client),
		joins:    make(chan net.Conn),
		control:  make(chan Message),
		incoming: make(chan Message),
		outgoing: make(chan Message),
	}
	server.Listen()
	return server
}

type Client struct {
	id         string
	connection *net.Conn
	incoming   chan Message
	outgoing   chan Message
	reader     *bufio.Reader
	writer     *bufio.Writer
	server     *Server
}

func (client *Client) Read() {
	data := make([]byte, CONN_MAX_SIZE)
	for {
		n, err := client.reader.Read(data)
		if (err == io.EOF) {
			fmt.Printf("Client disconnected\n")
			break
		} else if (err != nil) {
			fmt.Printf("Error reading from client, disconnecting...\n")
			break
		} else {
			client.incoming <- Message{MsgData, client.id, data[:n]}
		}
	}

	(*client.connection).Close()

	// Notify server of disconnect
	client.server.control <- Message{MsgDisconnect, client.id, []byte{}}
}

func (client *Client) Write() {
	for msg := range client.outgoing {
		_, err := client.writer.Write(msg.data)
		client.writer.Flush()
		if (err != nil) {
			fmt.Printf("Error writing to client\n")
			continue
		}
	}
	fmt.Printf("%s Write() exiting\n", client.id)
}

func (client *Client) Listen() {
	go client.Read()
	go client.Write()
}

func NewClient(connection net.Conn, server *Server) *Client {
	client := &Client{
		id:         connection.RemoteAddr().String(),
		connection: &connection,
		incoming:   make(chan Message),
		outgoing:   make(chan Message),
		reader:     bufio.NewReader(connection),
		writer:     bufio.NewWriter(connection),
		server:     server,
	}
	client.Listen()
	return client
}
