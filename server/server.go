package main

import (
	"bufio"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"net"
	"os"
	"time"
)

// Server parameters
const (
	CONN_HOST     = ""
	CONN_PORT     = "8191"
	CONN_TYPE     = "tcp"
	CONN_MAX_SIZE = 8191
)

// Message
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

type Server struct {
	clients  map[string]*Client
	joins    chan net.Conn
	incoming chan Message
	outgoing chan Message
	log      *bufio.Writer
}

func (server *Server) SetLogfile(logfile string) {
	fmt.Printf("Logging data to file \"%s\"\n", logfile)
	f, err := os.Create(logfile)
	if err != nil {
		fmt.Printf("Error creating log file\n")
		return
	}
	server.log = bufio.NewWriter(f)
}

func (server *Server) PrintClients() {
	fmt.Printf("[%d] Clients:\n", time.Now().Unix())
	if len(server.clients) > 0 {
		for k := range server.clients {
			callsign := server.clients[k].callsign
			if callsign != "" {
				fmt.Printf("    %s [%s]\n", k, callsign)
			} else {
				fmt.Printf("    %s\n", k)
			}
		}
	} else {
		fmt.Println("    None")
	}
}

func (server *Server) parsePacket(msg Message) {
	if *debug {
		datastr := hex.Dump(msg.data)
		fmt.Printf("[%d]\n%s\n", time.Now().Unix(), datastr)
	}
	if len(msg.data) < 2 {
		return
	}

	header := binary.LittleEndian.Uint16(msg.data[0:2])
	switch header {
	case 0xA02F: // GMSK header
		_, mycall := gmskParseHeader(msg)
		if server.clients[msg.sender] != nil {
			server.clients[msg.sender].callsign = mycall
		}
		server.Broadcast(msg)
	case 0xC012: // GMSK data
		//gmskParseData(msg)
		server.Broadcast(msg)
	default:
		// Everything else
	}
}

func (server *Server) Broadcast(msg Message) {
	for _, client := range server.clients {
		if msg.sender != client.id {
			client.outgoing <- msg
		} else {
			//fmt.Printf("rx from %s\n", client.id);
		}
	}
}

func (server *Server) Join(connection net.Conn) {
	client := NewClient(connection)
	server.clients[client.id] = client
	fmt.Printf("[%d] %s connected\n", time.Now().Unix(), client.id)
	server.PrintClients()
	go func() {
		for {
			server.incoming <- <-client.incoming
		}
	}()
}

func (server *Server) Disconnect(msg Message) {
	if msg.msgtype == MsgDisconnect {
		delete(server.clients, msg.sender)
		server.PrintClients()
	}
}

func (server *Server) Listen() {
	go func() {
		for {
			select {
			case msg := <-server.incoming:
				if msg.msgtype == MsgDisconnect {
					server.Disconnect(msg)
				} else if msg.msgtype == MsgData {
					server.parsePacket(msg)
					if server.log != nil {
						server.log.Write(msg.data)
						server.log.Flush()
					}
				}
			case conn := <-server.joins:
				server.Join(conn)
			}
		}
	}()
}

func (server *Server) Start() {
	fd, err := net.Listen(CONN_TYPE, CONN_HOST + ":" + CONN_PORT)
	if err != nil {
		fmt.Printf("Error listening on %s:%s: %s\n", CONN_HOST, CONN_PORT,
			err.Error())
		return
	}
	defer fd.Close()

	fmt.Println("Listening on " + CONN_HOST + ":" + CONN_PORT)
	for {
		conn, err := fd.Accept()
		if err != nil {
			fmt.Println("Error accepting connection: ", err.Error())
			return
		}
		server.joins <- conn
	}
}

func NewServer() *Server {
	server := &Server{
		clients:  make(map[string]*Client),
		joins:    make(chan net.Conn),
		incoming: make(chan Message),
		outgoing: make(chan Message),
	}
	server.Listen()
	return server
}

type Client struct {
	id         string
	callsign   string
	connection *net.Conn
	incoming   chan Message
	outgoing   chan Message
	reader     *bufio.Reader
	writer     *bufio.Writer
}

func (client *Client) Read() {
	data := make([]byte, CONN_MAX_SIZE)
	for {
		n, err := client.reader.Read(data)
		if err == io.EOF {
			fmt.Printf("[%d] %s disconnected\n", time.Now().Unix(), client.id)
			break
		} else if err != nil {
			fmt.Printf("Error reading from client, disconnecting...\n")
			break
		} else {
			client.incoming <- Message{MsgData, client.id, data[:n]}
		}
	}

	// Stop Write() for loop and close connection
	close(client.outgoing)
	(*client.connection).Close()

	// Notify server of disconnect
	client.incoming <- Message{MsgDisconnect, client.id, []byte{}}
}

func (client *Client) Write() {
	for msg := range client.outgoing {
		_, err := client.writer.Write(msg.data)
		client.writer.Flush()
		if err != nil {
			fmt.Printf("Error writing to client\n")
			continue
		}
	}
}

func NewClient(connection net.Conn) *Client {
	client := &Client{
		id:         connection.RemoteAddr().String(),
		connection: &connection,
		incoming:   make(chan Message),
		outgoing:   make(chan Message),
		reader:     bufio.NewReader(connection),
		writer:     bufio.NewWriter(connection),
	}

	// Start read and write threads
	go client.Read()
	go client.Write()

	return client
}
