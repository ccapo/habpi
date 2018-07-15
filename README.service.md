# Systemd Service Setup

1. Create service script:

`sudo vi /lib/systemd/system/<program_name>.service`

2. Include the following information:

[Unit]
Description=<program_name>
After=network.target

[Service]
Type=forking
ExecStart=/path/to/<program_name>
KillMode=mixed
RestartSec=5
Restart=on-failure

[Install]
WantedBy=multi-user.target

3. Stiplulate what other target should precede <program_name> in `After=` (i.e. `gpsd`)

4. Enable script:

`sudo systemctl enable <program_name>`

5. Reboot

FYI:

- To start, stop, restart the service:

`sudo systemctl [start|stop|restart] <program_name>`
