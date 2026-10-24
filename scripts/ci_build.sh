#!/bin/bash

./scripts/download_dependencies.sh
source ./scripts/env_vars.sh
./build-scripts/ps5-payload-sdk.sh
