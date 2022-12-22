# getting the base image
FROM ubuntu

# image maintainer
MAINTAINER Matheus Souza <mfbsouza.it@gmail.com>

# setup dependencies for this image
RUN apt-get update && \
	apt-get install -y -q --no-install-recommends file git gcc g++ make automake autoconf libtool

## clone cpputest
WORKDIR /repo
RUN git config --global http.sslverify false
RUN git clone https://github.com/cpputest/cpputest.git

## build cpputest and set env variable
WORKDIR /repo/cpputest
RUN autoreconf . -i && ./configure && make tdd -j$(nproc)
ENV CPPUTEST_HOME=/repo/cpputest

# run
WORKDIR /app
CMD echo "hello from docker image!"
