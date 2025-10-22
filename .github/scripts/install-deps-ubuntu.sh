#!/bin/bash
set -e

MAX_ATTEMPTS=3
ATTEMPT=0

install_packages() {
    local apt_args=(
        ${CI:+-y}
        --no-install-recommends
        --fix-missing
        -o APT::Acquire::Retries=3
        -o APT::Acquire::http::Timeout=30
        -o APT::Acquire::https::Timeout=30
    )

    if [[ "${_loglevel:-1}" == "0" ]]; then
        apt_args+=(--quiet)
    fi

    sudo apt-get "${apt_args[@]}" "$@"
}

# Update with retry logic
while [ $ATTEMPT -lt $MAX_ATTEMPTS ]; do
    if timeout 300 sudo apt-get update -qq; then
        break
    fi
    ATTEMPT=$((ATTEMPT + 1))
    echo "Update attempt $ATTEMPT failed, retrying..."
    sleep 5
done

# Install packages with retry
ATTEMPT=0
while [ $ATTEMPT -lt $MAX_ATTEMPTS ]; do
    if install_packages install "$@"; then
        exit 0
    fi
    ATTEMPT=$((ATTEMPT + 1))
    echo "Installation attempt $ATTEMPT failed, retrying..."
    sudo apt-get clean
    sleep 10
done

echo "Failed to install packages after $MAX_ATTEMPTS attempts"
exit 1
