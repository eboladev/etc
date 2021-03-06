���ꠎ�䚰C:$Z� 0��c��>pE S�9 �$*>Я��Y����HL�zHa , �~���0��l:~���t��a@r@
��ؠ@
�3�>PHc���]���D�p< RS5 �_ڬi�r0T`Mۧ3@��D��+p�c��`������i@m@~���J���;�����'�{��CJ��k3�m��>0��_���k��$R> [P9����a��+��:>@��֋��3�� 	�	� 	����_@�K�;�~�ܷ���;�r`�̷�<�L�|Q��[����YмC�t0�[c@�Gk�,t����P	� � 	���|��>����������|�@����D� #�ڹK<����F\���*�+G�Ö0��J\�j|�B̿|���K<�l��Bş:�@̷���U�,�QȆ������n�Ǌ 5��5<5�Ǘ|Ȝ��A�0��
�\ʦ|�	���<ʨ,ƅ>�aʰ�
qD�ʟZ�\&
��	�˰l��>�0��L�˞<̳�����|̟���,��|Ϳ���,Ҍ�`��?����Q��<����l��1���u��a̢����<������� �S�l�����]�r���lˡlt

	set result $line
	while {1} {
	    if {[gets $socket line] < 0} {
		error "unexpected end of connection"
	    }
	    append result $line
	    switch -glob -- $line "$code *" {
		break
	    }
	}
    } default {
	error "bad reply: $line"
    }

    switch -glob -- $code {[123]??} {
	return $result
    } else {
	return -code error $result
    }
}

# Send FTP command $cmd on $socket, and get reply.  Result contains
# the entire reply message.  The first three bytes contains the result
# code.

proc ftp_Cmd {socket cmd} {
    global ftp
    if {$ftp(debug)} {puts stderr $cmd}
    puts $socket $cmd
    ftp_GetReply $socket
}

# Connect to ftp server on machine $host (port number $port) with
# username $user and password $pass.  Returns channel for the
# connection.

proc ftp_Connect { host user { pass {} } { port 21 } } {
    # Connect and get initial reply message

    set chan [socket $host $port]
    fconfigure $chan -buffering line -translation crlf

    if { [catch {
	ftp_GetReply $chan

	# Login

	switch -glob -- [ftp_Cmd $chan "USER $user"] {
	    {331 *} { ftp_Cmd $chan "PASS $pass" }
	}

	# Configure transfers for file streams.

	ftp_Cmd $chan "STRU F"
	ftp_Cmd $chan "MODE S"
    } msg] } {
	close $chan
	error $msg
    }

    return $chan
}

# Open data connection for FTP control connection on channel $chan.
# Data type, $type, can be ascii or binary.  Save info on the data
# channel in global array name $chan.

proc ftp_OpenData { chan type mode } {
    global ftp_types
    upvar #0 $chan var

    ftp_Cmd $chan "TYPE $ftp_types($type)"

    switch -- $mode passive {
	set result [ftp_Cmd $chan PASV]
	if { ![regexp {([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+)} \
		$result junk h1 h2 h3 h4 p1 p2] } {
	    error $result
	}
	set host "$h1.$h2.$h3.$h4"
	set port [expr {$p1*256 + $p2}]

	set dchan [socket $host $port]
	ftp_ConfigData $dchan $type
    } active {
	set port 10000
	while [catch { socket -server "ftp_ActiveConn $chan" $port }] {
	    incr port
	    if { $port > 10100} {
		error {can't open server socket for active data connections}
	    }
	}
	regsub -all {\.} [lindex [fconfigure $chan -sockname] 0] {,} addr
	append addr ",[expr { $port/256 }],[expr { $port%256 }]"
	ftp_Cmd $chan "PORT $addr"
	set dchan {}
    }
    set var(dchan) $dchan
    set var(type) $type
}

# Configure FTP data-channel for $type transfers, $type can be ascii
# or binary.

proc ftp_ConfigData { dchan type } {
    fconfigure $dchan -blocking 1
    switch -- $type {
	ascii { fconfigure $dchan -buffering line -translation crlf }
	binary { fconfigure $dchan -buffering full -translation binary }
    }
}

# Callback for active data-channel connection.  Configures the channel
# for the appropriate data type (ascii or binary).

proc ftp_ActiveConn { chan dchan addr port } {
    upvar #0 $chan var
    ftp_ConfigData $dchan $var(type)
    set var(dchan) $dchan
}

# Retrieve data from data-channel of FTP connection $chan, then close
# connection.  Returns the data read.

proc ftp_GetData { chan } {
    upvar #0 $chan var
    switch -- $var(dchan) {} { vwait ${chan}(dchan) }
    set data [read $var(dchan)]
    close $var(dchan)
    return $data
}

# Send data in $data over data-channel of FTP connection $chan, then close
# connection.

proc ftp_PutData { chan data } {
    upvar #0 $chan var
    switch -- $var(dchan) {} { vwait ${chan}(dchan) }
    puts -nonewline $var(dchan) $data
    close $var(dchan)
}

# Get directory listing from FTP connection $chan.  Use $mode (active
# or passive) for data connection.

proc ftp_List { chan {mode passive} } {
    ftp_OpenData $chan A $mode
    ftp_Cmd $chan LIST
    set data [ftp_GetData $chan]
    ftp_GetReply $chan
    return $data
}

# Get file $filename via FTP connection $chan.  Transfer data as type
# $type (can be "ascii" (default) or "binary").  Returns contents of
# file.  Use $mode (active or passive) for data connection.

proc ftp_Get { chan filename {type ascii} {mode passive} } {
    ftp_OpenData $chan $type $mode
    ftp_Cmd $chan "RETR $filename"
    set data [ftp_GetData $chan]
    ftp_GetReply $chan
    return $data
}

# Store $data in file $filename via FTP connection $chan.  Transfer data
# as type $type (can be "ascii" (default) or "binary").
# Use $mode (active or passive) for data connection.

proc ftp_Put { chan filename data {type ascii} {mode passive} } {
    ftp_OpenData $chan $type $mode
    ftp_Cmd $chan "STOR $filename"
    ftp_PutData $chan $data
    ftp_GetReply $chan
}

# Change directory for FTP connection $chan to $dirname.

proc ftp_ChangeDir { chan dirname } {
    ftp_Cmd $chan "CWD $dirname"
}
