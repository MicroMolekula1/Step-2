#!/usr/bin/env bash
set -euo pipefail

docker build -t libcaesar-env .
docker run --rm -t -v "$PWD":/work -w /work libcaesar-env make clean all test
