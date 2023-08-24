FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y && \
	rm -rf /var/lib/apt/lists/*

COPY build/MimicWindowsServer /usr/bin/MimicWindowsServer

EXPOSE 50055
CMD ["/usr/bin/MimicWindowsServer"]
