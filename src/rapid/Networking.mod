MODULE Networking(SYSMODULE)

    RECORD Packet
        num cmd;
        num x;
        num y;
        num z;
        num rot_z;
    ENDRECORD

    VAR Packet pkt;
    VAR socketdev server_socket;
    VAR socketdev client_socket;
    VAR bool server_created:=FALSE;

    ! 0     = ABB_CMD_POSITION:            We expect a x,y,z position in robot coordinates, 4 bytes per float.
    ! 1     = ABB_CMD_TOGGLE_IO:           Change the value of an I/O port. 
    ! 2     = ABB_CMD_RESET_PACKET_INDEX:  Reset the read and write indices into our packet array.
    ! 3     = ABB_CMD_DRAW:                When we receive this command we iterate over the `packets` and move the tcp.
    ! 255   = unset/unknown command.
    VAR pos read_position;
    VAR byte command:=255;
    VAR num read_offset:=1;
    VAR num bytes_available:=0;
    VAR rawbytes raw_data_in;

    PERS Packet packets{500}:=[[0,0,-680,-300,0],[0,0,0,0,0],[0,0,-680,200,0],[1,2,1,0,0],[0,0,0,-50,120],[1,2,0,0,0],[1,2,1,0,0],[0,0,680,200,120],[1,2,0,0,0],[1,1,1,0,0],[0,0,680,200,-120],[1,1,0,0,0],[1,1,1,0,0],[0,0,0,-50,20],[1,1,0,0,0],[5,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0]];
    PERS num pkt_read_dx:=17;
    PERS num pkt_write_dx:=1;
    PERS bool has_data:=FALSE;
    PERS bool drawing_ready:=FALSE;

    ! Experimental
    ! n = none, starting up.
    ! c = someone just connected
    ! r = ready to receive commands
    ! d = we're currently drawing
    PERS string state:="r";

    VAR socketstatus server_ss;
    VAR socketstatus client_ss;
    VAR bool client_ok:=FALSE;
    VAR bool server_ok:=FALSE;

    PROC main()

        IF server_created=FALSE THEN

            state:="n";
            pkt_write_dx:=1;
            has_data:=FALSE;
            drawing_ready:=FALSE;

            FOR i FROM 1 TO 500 DO
                packets{i}:=[0,0,0,0,0];
            ENDFOR

            createListeningSocket;

        ENDIF


        client_ss:=SocketGetStatus(client_socket);
        IF client_ss<>SOCKET_CONNECTED THEN
            TPWrite "Client not connected";
        ENDIF

        bytes_available:=SocketPeek(client_socket);

        IF bytes_available>=1 THEN

            ! @todo how to deal with situations where the buffer has more then 1024 bytes ?
            ! Append read data to our parse buffer. 
            ClearRawBytes raw_data_in;
            SocketReceive client_socket\RawData:=raw_data_in;

            read_offset:=1;
            bytes_available:=RawBytesLen(raw_data_in);

            WHILE bytes_available>0 DO

                IF bytes_available>=1 THEN

                    UnpackRawBytes raw_data_in,read_offset,command\Hex1;

                    IF 0=command AND bytes_available>=13 THEN
                        ! Read a position.      
                        UnpackRawBytes raw_data_in\Network,read_offset+1,pkt.x\Float4;
                        UnpackRawBytes raw_data_in\Network,read_offset+5,pkt.y\Float4;
                        UnpackRawBytes raw_data_in\Network,read_offset+9,pkt.z\Float4;
                        UnpackRawBytes raw_data_in\Network,read_offset+13,pkt.rot_z\Float4;

                        ! @todo - we could just directly read into the correct location instead of copying the data;
                        packets{pkt_write_dx}.cmd:=0;
                        packets{pkt_write_dx}.x:=pkt.x;
                        packets{pkt_write_dx}.y:=pkt.y;
                        packets{pkt_write_dx}.z:=pkt.z;
                        packets{pkt_write_dx}.rot_z:=pkt.rot_z;

                        pkt_write_dx:=pkt_write_dx+1;
                        bytes_available:=bytes_available-17;
                        read_offset:=read_offset+17;

                        ! Signal the other task we have data.
                        IF pkt_write_dx>500 THEN
                            pkt_write_dx:=1;
                        ENDIF

                    ELSEIF 1=command AND bytes_available>=9 THEN

                        ! We read 2 bytes, 1: what i/o port, 2: on/off.  
                        UnpackRawBytes raw_data_in\Network,read_offset+1,read_position.x\Float4;
                        UnpackRawBytes raw_data_in\Network,read_offset+5,read_position.y\Float4;

                        packets{pkt_write_dx}.cmd:=1;
                        packets{pkt_write_dx}.x:=read_position.x;
                        packets{pkt_write_dx}.y:=read_position.y;
                        packets{pkt_write_dx}.z:=0;

                        pkt_write_dx:=pkt_write_dx+1;
                        bytes_available:=bytes_available-9;
                        read_offset:=read_offset+9;

                    ELSEIF 2=command THEN
                        ! @todo - not sure if we really need this, probably not.
                        bytes_available:=bytes_available-1;
                        read_offset:=read_offset+1;
                    ELSEIF 3=command THEN
                        ! Notify the client that we're drawing.
                        setStateDrawing;

                        ! The main task waits for this flag.
                        bytes_available:=bytes_available-1;
                        read_offset:=read_offset+1;
                        has_data:=TRUE;

                        ! The main task sets this to true when ready.
                        WaitUntil drawing_ready;
                        drawing_ready:=FALSE;

                        pkt_write_dx:=1;

                        ! Notify the client again that we're ready 
                        setStateReady;

                    ELSEIF 4=command THEN
                        
                        SocketSend client_socket\Str:=state;
                        read_offset:=read_offset+1;
                        bytes_available:=bytes_available-1;
                        
                    ELSEIF 5=command THEN
                        
                        ! Move back to home position (original state).
                        packets{pkt_write_dx}.cmd:=5;
                        pkt_write_dx:=pkt_write_dx+1;
                        read_offset:=read_offset+1;
                        bytes_available:=bytes_available-1;

                    ELSE
                        ! Reset 
                        bytes_available:=0;
                        read_offset:=0;
                        ClearRawBytes raw_data_in;
                    ENDIF
                ENDIF
            ENDWHILE
        ENDIF
    ERROR
        TEST ERRNO
        CASE ERR_SOCK_TIMEOUT:
            TPWrite "Socket timeout! @todo handle this error correctly";
        CASE ERR_SOCK_CLOSED:
            TPWrite "ERROR: Socket closed";
            SocketClose client_socket;
            SocketClose server_socket;
            checkServerAndClientSockets;
            RETRY;
        DEFAULT:
            TPWrite "Unhandled error: "\Num:=ERRNO;
        ENDTEST
    ENDPROC

    PROC notifyCurrentState()

        checkServerAndClientSockets;

        IF client_ok<>TRUE OR server_ok<>TRUE THEN
            RETURN ;
        ENDIF

        client_ss:=SocketGetStatus(client_socket);

        SocketSend client_socket\Str:=state;

        ! We need to handle an ERROR here, because it's not always possible 
        ! for sockets to know that they're not valid anymore until we send something.
        ! (or at least with posix sockets; I suspect these are used by RAPID too).
    ERROR
        TEST ERRNO
        CASE ERR_SOCK_TIMEOUT:
            TPWrite "Socket timeout! @todo handle this error correctly";
        CASE ERR_SOCK_CLOSED:
            TPWrite "ERROR: Socket closed";
            SocketClose client_socket;
            SocketClose server_socket;
            checkServerAndClientSockets;
            RETRY;
        DEFAULT:
            TPWrite "Unhandled error: "\Num:=ERRNO;
        ENDTEST

    ENDPROC

    PROC setStateReady()
        state:="r";
        notifyCurrentState;
    ENDPROC

    PROC setStateDrawing()
        state:="d";
        notifyCurrentState;
    ENDPROC

    PROC checkServerAndClientSockets()

        ! Check server.
        server_ss:=SocketGetStatus(server_socket);
        IF server_ss<>SOCKET_CREATED AND server_ss<>SOCKET_LISTENING THEN
            TPWrite "Server socket not created or closed. (Re)Initializing.";
            createListeningSocket;
        ENDIF

        ! When the client isn't connected but was creaded, make sure it will be recreated. 
        client_ss:=SocketGetStatus(client_socket);
        IF client_ss<>SOCKET_CONNECTED THEN
            IF client_ss=SOCKET_CREATED THEN
                SocketClose client_socket;
            ENDIF
            client_ok:=FALSE;
        ELSE
            client_ok:=TRUE;
        ENDIF

    ENDPROC


    PROC createListeningSocket()

        server_ss:=SocketGetStatus(server_socket);

        IF server_ss<>SOCKET_CREATED THEN
            SocketCreate server_socket;
            !SocketBind server_socket,"127.0.0.1",1025;
            !SocketBind server_socket, "192.168.125.1", 1025;
            SocketBind server_socket,"192.168.1.100",1025;
            SocketListen server_socket;
            SocketAccept server_socket,client_socket\Time:=WAIT_MAX;
            TPWrite "Accepted a new connection";
            server_created:=TRUE;
            drawing_ready:=FALSE;
            state:="r";
            server_ok:=TRUE;
            TPWrite "SocketStatus: "+ValToStr(server_ss);
        ENDIF

        client_ss:=SocketGetStatus(client_socket);

        IF client_ss=SOCKET_CREATED THEN
            TPWrite "Client socket already created.";
        ELSEIF client_ss=SOCKET_CONNECTED THEN
            TPWrite "Client socket connected";
        ELSE
            TPWrite "Unhandled socket state: "+ValToStr(client_ss);
        ENDIF

    ERROR
        TEST ERRNO
        CASE ERR_SOCK_TIMEOUT:
            TPWrite "ERROR: Socket timeout";
            SocketClose client_socket;
            SocketClose server_socket;
            checkServerAndClientSockets;
            RETRY;
        CASE ERR_SOCK_CLOSED:
            TPWrite "ERROR: Socket closed";
            SocketClose client_socket;
            SocketClose server_socket;
            checkServerAndClientSockets;
            RETRY;
        DEFAULT:
            TPWrite "Unhandled error: "\Num:=ERRNO;
        ENDTEST
    ENDPROC


    PROC setStateToReadyWithDrawing()
        state:="r";
    ENDPROC
ENDMODULE