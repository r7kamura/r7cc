FROM ubuntu:19.10

WORKDIR /app

RUN apt update && \
  apt install --no-install-recommends --yes build-essential && \
  rm -rf /var/lib/apt/lists/*
