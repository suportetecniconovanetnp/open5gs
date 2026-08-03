# Open5GS Portainer Stack Examples

Dois exemplos de stack para Portainer, separados em:

- `Open5GS Control Plane`: `sgwc`, `scp`, `nrf`, `nssf`
- `Open5GS User Plane`: `sgwu`, `upf`

Os exemplos abaixo seguem o mesmo padrão do seu compose:

- `network_mode: "host"`
- arquivos de configuração montados em `/etc/open5gs`
- logs montados em `/var/log/open5gs`
- `TZ: America/Belem`
- `restart: unless-stopped`

## Open5GS Control Plane

```yaml
services:
  sgwc:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    volumes:
      - /var/log/open5gs/sgwc.log:/var/log/open5gs/sgwc.log:rw
      - /opt/open5gs/etc/sgwc.yaml:/etc/open5gs/sgwc.yaml:ro
    environment:
      TZ: America/Belem
    command: ["/usr/local/bin/open5gs-sgwcd", "-c", "/etc/open5gs/sgwc.yaml"]
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"

  scp:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    volumes:
      - /var/log/open5gs/scp.log:/var/log/open5gs/scp.log:rw
      - /opt/open5gs/etc/scp.yaml:/etc/open5gs/scp.yaml:ro
    environment:
      TZ: America/Belem
    command: ["/usr/local/bin/open5gs-scpd", "-c", "/etc/open5gs/scp.yaml"]
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"

  nrf:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    volumes:
      - /var/log/open5gs/nrf.log:/var/log/open5gs/nrf.log:rw
      - /opt/open5gs/etc/nrf.yaml:/etc/open5gs/nrf.yaml:ro
    environment:
      TZ: America/Belem
    command: ["/usr/local/bin/open5gs-nrfd", "-c", "/etc/open5gs/nrf.yaml"]
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"

  nssf:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    volumes:
      - /var/log/open5gs/nssf.log:/var/log/open5gs/nssf.log:rw
      - /opt/open5gs/etc/nssf.yaml:/etc/open5gs/nssf.yaml:ro
    environment:
      TZ: America/Belem
    command: ["/usr/local/bin/open5gs-nssfd", "-c", "/etc/open5gs/nssf.yaml"]
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"
```

## Open5GS User Plane

```yaml
services:
  sgwu:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    cap_add:
      - NET_ADMIN
    devices:
      - /dev/net/tun:/dev/net/tun
    volumes:
      - /var/log/open5gs/sgwu.log:/var/log/open5gs/sgwu.log:rw
      - /opt/open5gs/etc/sgwu.yaml:/etc/open5gs/sgwu.yaml:ro
    environment:
      TZ: America/Belem
    command:
      - /bin/bash
      - -lc
      - exec /usr/local/bin/open5gs-sgwud -c /etc/open5gs/sgwu.yaml
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"

  upf:
    image: open5gs/ubuntu-latest-open5gs-build:20260801
    restart: unless-stopped
    network_mode: "host"
    cap_add:
      - NET_ADMIN
    devices:
      - /dev/net/tun:/dev/net/tun
    volumes:
      - /var/log/open5gs/upf.log:/var/log/open5gs/upf.log:rw
      - /opt/open5gs/etc/upf.yaml:/etc/open5gs/upf.yaml:ro
    environment:
      TZ: America/Belem
    command:
      - /bin/bash
      - -lc
      - exec /usr/local/bin/open5gs-upfd -c /etc/open5gs/upf.yaml
    logging:
      driver: local
      options:
        max-size: "10m"
        max-file: "3"
```

## Observações

- Se o seu `sgwu` ou `upf` depender de preparação extra de rede, você pode trocar o `command` por algo como:

```yaml
command:
  - /bin/bash
  - -lc
  - /root/setup.sh; exec /usr/local/bin/open5gs-sgwud -c /etc/open5gs/sgwu.yaml
```

- Em Portainer, cole cada bloco YAML em uma stack separada.
- Como os exemplos usam `network_mode: "host"`, confira conflito de portas antes de subir os serviços no mesmo host.
