# Use Ubuntu 20.04 as the base image
FROM ubuntu:20.04

# Set non-interactive mode for package installation
ENV DEBIAN_FRONTEND=noninteractive

# Define environment variables
ENV QT_VERSION=6.8.0
ENV QT_COMPILER=linux_gcc_64
ENV QT_COMPILER_DIR=gcc_64
ENV BUILD_TYPE=Release
ENV BUILD_APP_PROJECT=true
ENV BUILD_TEST_PROJECT=true
ENV $RUN_APP_ON_CONTAINER_START=false
ENV THIRD_PARTY_INCLUDE_DIR=/home/user/ThirdParty
ENV PROJECT_NAME=QtTemplate
ENV QT_QPA_PLATFORM=xcb
ENV QT_DEBUG_PLUGINS=1

# Install required system packages and CMake 3.28.0-rc3
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    git \
    libgl1-mesa-dev \
    libxcb-cursor0 \
    libxcb-cursor-dev \
    libx11-xcb-dev \
    libxrender1 \
    libxi6 \
    libxrandr2 \
    libxcomposite1 \
    libxcursor1 \
    libxtst6 \
    libxdamage1 \
    fonts-dejavu-core \
    xvfb \
    unzip \
    python3 \
    python3-pip \
    python3-dev \
    libglib2.0-0 \
    libglib2.0-dev \
    libxkbcommon0 \
    libxkbcommon-x11-0 \
    libfontconfig1 \
    libdbus-1-3 \
    libxcb-xfixes0 \
    libxcb-shape0 \
    libxcb-shm0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-render0 \
    libxcb-xinerama0 \
    locales \
    || exit 1 \
    && wget https://github.com/Kitware/CMake/releases/download/v3.28.0-rc3/cmake-3.28.0-rc3-linux-x86_64.sh -O /tmp/cmake.sh \
    && chmod +x /tmp/cmake.sh \
    && /tmp/cmake.sh --skip-license --prefix=/usr/local || exit 1

# Configure UTF-8 locale
RUN locale-gen en_US.UTF-8 || exit 1
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# Install aqtinstall for Qt installation
RUN python3 -m pip install --no-cache-dir aqtinstall || exit 1

# Install Qt using aqtinstall
RUN python3 -m aqt install-qt linux desktop ${QT_VERSION} ${QT_COMPILER} --outputdir /opt/Qt || exit 1

# Set the QT6_DIR environment variable
ENV QT6_DIR=/opt/Qt/${QT_VERSION}/${QT_COMPILER_DIR}
ENV PATH="${QT6_DIR}/bin:${PATH}"

# Create a working directory
WORKDIR /app

# Copy the source code into the container
COPY . .

# Create the ThirdParty directory
RUN mkdir -p ${THIRD_PARTY_INCLUDE_DIR} || exit 1

# Build the app project (if enabled)
RUN if [ "$BUILD_APP_PROJECT" = "true" ]; then \
    cmake -B _build_app_release -S . \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DTHIRD_PARTY_INCLUDE_DIR=${THIRD_PARTY_INCLUDE_DIR} \
        -DMAIN_PROJECT_NAME=${PROJECT_NAME} || exit 1 \
    && cmake --build _build_app_release --config ${BUILD_TYPE} || exit 1; \
    fi

# Build the test project (if enabled)
RUN if [ "$BUILD_TEST_PROJECT" = "true" ]; then \
    cmake -B _build_tests_release -S . \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -D${PROJECT_NAME}_BUILD_TARGET_TYPE=static_library \
        -D${PROJECT_NAME}_BUILD_TEST_PROJECT=true \
        -DTHIRD_PARTY_INCLUDE_DIR=${THIRD_PARTY_INCLUDE_DIR} \
        -DMAIN_PROJECT_NAME=${PROJECT_NAME} || exit 1 \
    && cmake --build _build_tests_release --config ${BUILD_TYPE} || exit 1; \
    fi

# Run tests (if enabled)
RUN if [ "$BUILD_TEST_PROJECT" = "true" ]; then \
    Xvfb :99 -screen 0 1920x1080x24 -nolisten tcp & \
    export DISPLAY=:99 && \
    sleep 3 && \
    ./_build_tests_release/QT_Project_Tests/${PROJECT_NAME}_Tests || exit 1; \
    fi

# Default command: Conditionally run the app executable
CMD if [ "$RUN_APP_ON_CONTAINER_START" = "true" ] && [ "$BUILD_APP_PROJECT" = "true" ]; then \
        Xvfb :99 -screen 0 1920x1080x24 -nolisten tcp & \
        export DISPLAY=:99 && \
        ./_build_app_release/QT_Project/QtTemplate; \
    else \
        echo "No app project to run"; \
    fi
