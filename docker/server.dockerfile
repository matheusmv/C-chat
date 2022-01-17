FROM debian:11

# Install basic dependencies
RUN apt-get update -y && apt-get install -y gcc make

# Setup project
WORKDIR /app
COPY . .
RUN make

ENV DEFAULT_PORT=8888

RUN echo "#!/bin/bash \n ./server -p $DEFAULT_PORT" > ./run.sh
RUN chmod +x ./run.sh

ENTRYPOINT [ "./run.sh" ]
