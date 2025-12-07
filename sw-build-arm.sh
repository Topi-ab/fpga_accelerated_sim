#!/bin/bash

HOST_UID=$(id -u) HOST_GID=$(id -g) docker-compose run --rm build-arm64
