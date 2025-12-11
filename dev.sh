#!/bin/bash

# Development script for OBS EmuLens plugin
# Usage: ./dev.sh [clean|build|run|package|all]

set -e

PLUGIN_NAME="obs-emulens"
BUILD_DIR="${BUILD_DIR:-build}"
OBS_INSTALL_PATH="${OBS_INSTALL_PATH:-/Applications/OBS.app/Contents/PlugIns}"

# Determine platform
PLATFORM="$(uname -s)"
case "${PLATFORM}" in
    Darwin*)
        CONFIG_PRESET="macos"
        PLUGIN_EXT=".dylib"
        OBS_PLUGIN_PATH="${OBS_INSTALL_PATH}/obs-plugins"
        ;;
    Linux*)
        CONFIG_PRESET="ubuntu-x86_64"
        PLUGIN_EXT=".so"
        OBS_PLUGIN_PATH="${HOME}/.config/obs-studio/plugins/${PLUGIN_NAME}/bin/64bit"
        ;;
    MINGW*|MSYS*|CYGWIN*|Windows_NT)
        CONFIG_PRESET="windows-x64"
        PLUGIN_EXT=".dll"
        OBS_PLUGIN_PATH="${OBS_INSTALL_PATH}/obs-plugins/64bit"
        ;;
    *)
        echo "Unsupported platform: ${PLATFORM}"
        exit 1
        ;;
esac

# Build configuration
BUILD_CONFIG="RelWithDebInfo"

# Clean build directory
clean() {
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
}

# Configure and build the project
build() {
    echo "Configuring and building ${PLUGIN_NAME}..."
    cmake --preset "${CONFIG_PRESET}" \
        -DCMAKE_BUILD_TYPE="${BUILD_CONFIG}" \
        -DENABLE_FRONTEND_API=ON \
        -DENABLE_QT=OFF
    
    cmake --build --preset "${CONFIG_PRESET}" --config "${BUILD_CONFIG}"
    echo "Build complete!"
}

# Install the plugin to OBS
install() {
    echo "Installing ${PLUGIN_NAME} to OBS..."
    local plugin_file="${BUILD_DIR}/${PLUGIN_NAME}${PLUGIN_EXT}"
    
    if [ ! -f "${plugin_file}" ]; then
        echo "Error: Plugin not found at ${plugin_file}"
        exit 1
    fi
    
    mkdir -p "${OBS_PLUGIN_PATH}"
    cp "${plugin_file}" "${OBS_PLUGIN_PATH}/"
    
    # Copy data files if they exist
    if [ -d "data" ]; then
        echo "Copying data files..."
        local target_data_dir="${OBS_PLUGIN_PATH}/../${PLUGIN_NAME}"
        
        if [[ "${PLATFORM}" == "Linux"* ]]; then
            target_data_dir="${OBS_PLUGIN_PATH}/../../data"
        fi

        mkdir -p "${target_data_dir}"
        cp -r data/* "${target_data_dir}/"
    fi
    
    echo "Installation complete!"
}

# Run OBS with the plugin
run() {
    echo "Starting OBS..."
    case "${PLATFORM}" in
        Darwin*)
            open -n "/Applications/OBS.app" --args --verbose
            ;;
        Linux*)
            obs --verbose
            ;;
        MINGW*|MSYS*|CYGWIN*|Windows_NT)
            start "" "C:\\Program Files\\obs-studio\\bin\\64bit\\obs64.exe" --verbose
            ;;
    esac
}

# Package the plugin for distribution
package() {
    echo "Packaging ${PLUGIN_NAME}..."
    local package_dir="${BUILD_DIR}/package/${PLUGIN_NAME}"
    
    mkdir -p "${package_dir}"
    
    # Copy plugin binary
    cp "${BUILD_DIR}/${PLUGIN_NAME}${PLUGIN_EXT}" "${package_dir}/"
    
    # Copy data files
    if [ -d "data" ]; then
        cp -r data "${package_dir}/"
    fi
    
    # Copy license and readme
    cp LICENSE README.md "${package_dir}/"
    
    # Create zip/tar archive
    pushd "${BUILD_DIR}" > /dev/null
    if [ "${PLATFORM}" = "Darwin" ]; then
        zip -r "${PLUGIN_NAME}-${PLATFORM}.zip" "${PLUGIN_NAME}"
    else
        tar -czf "${PLUGIN_NAME}-${PLATFORM}.tar.gz" "${PLUGIN_NAME}"
    fi
    popd > /dev/null
    
    echo "Package created at ${BUILD_DIR}/${PLUGIN_NAME}-${PLATFORM}.*"
}

# Main script
case "$1" in
    clean)
        clean
        ;;
    build)
        build
        ;;
    install)
        install
        ;;
    run)
        run
        ;;
    package)
        package
        ;;
    all)
        clean
        build
        install
        run
        ;;
    *)
        echo "Usage: $0 [clean|build|install|run|package|all]"
        exit 1
        ;;
esac

exit 0
