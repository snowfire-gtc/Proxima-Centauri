# Руководство по развёртыванию Proxima & Centauri

## Версия 1.0.0

## Содержание

1. [Требования](#требования)
2. [Сборка компилятора](#сборка-компилятора)
3. [Сборка IDE](#сборка-ide)
4. [Установка](#установка)
5. [Развёртывание в production](#развёртывание-в-production)
6. [CI/CD](#cicd)
7. [Устранение проблем](#устранение-проблем)

## Требования

### Минимальные

- **ОС:** Linux (Ubuntu 20.04+), macOS 10.15+, Windows 10+
- **Процессор:** x86_64, 2 ядра
- **Память:** 4 GB RAM
- **Диск:** 2 GB свободного места
- **CMake:** 3.15+
- **C++ Compiler:** GCC 9+, Clang 10+, MSVC 2019+

### Для полной функциональности

- **LLVM:** 14.0+
- **Qt:** 5.15.2+
- **CUDA:** 11.0+ (опционально)
- **Git:** 2.0+

## Сборка компилятора

### Linux

```bash
cd proxima-compiler
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DENABLE_CUDA=OFF \
    -DLLVM_DIR=/usr/lib/llvm-14/cmake

make -j$(nproc)
make test
sudo make install

### MacOS

``` bash
cd proxima-compiler
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DLLVM_DIR=/usr/local/opt/llvm/cmake

make -j$(sysctl -n hw.ncpu)
make test
sudo make install
```

### Windows

``` powershell
cd proxima-compiler
mkdir build
cd build

cmake .. `
    -G "Visual Studio 16 2019" `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_TESTS=ON `
    -DLLVM_DIR="C:\Program Files\LLVM\cmake"

cmake --build . --config Release
ctest -C Release
```

## Сборка IDE

### Linux

``` bash
cd centauri-ide
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/Qt5.15.2

make -j$(nproc)
sudo make install
```

### macOS

``` bash
cd centauri-ide
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5

make -j$(sysctl -n hw.ncpu)

# Создать app bundle
make package
```

### Windows

``` powershell
cd centauri-ide
mkdir build
cd build

cmake .. `
    -G "Visual Studio 16 2019" `
    -DCMAKE_PREFIX_PATH="C:\Qt\5.15.2\msvc2019_64"

cmake --build . --config Release

# Создать installer
cpack -G NSIS
```

## Установка

### Linux (DEB)

``` bash
sudo dpkg -i proxima-centauri_1.0.0_amd64.deb
sudo apt-get install -f  # Install dependencies
```

### Linux (RPM)

``` bash
sudo rpm -ivh proxima-centauri-1.0.0.x86_64.rpm
```

### macOS

``` bash
sudo installer -pkg ProximaCentauri-1.0.0.pkg -target /
```

### Windows

``` powershell
# Запустить installer
ProximaCentauri-1.0.0-win64.exe
```

### Из исходников

``` bash
# Скомпилировать
./install.sh install

# Проверить установку
proxima --version
centauri --version
```

## Развёртывание в production

### Серверная установка

``` bash
# Создать пользователя
sudo useradd -r proxima
sudo mkdir -p /opt/proxima
sudo chown proxima:proxima /opt/proxima

# Установить
sudo tar -xzf proxima-centauri-1.0.0.tar.gz -C /opt/proxima
sudo ln -s /opt/proxima/bin/proxima /usr/bin/proxima
sudo ln -s /opt/proxima/bin/centauri /usr/bin/centauri

# Настроить права
sudo chmod 755 /opt/proxima/bin/*
sudo chmod -R 755 /opt/proxima/share
```

### Docker контейнер

``` dockerfile
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    llvm-14-dev \
    qt5-default \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

COPY proxima-centauri-1.0.0.tar.gz /tmp/
RUN tar -xzf /tmp/proxima-centauri-1.0.0.tar.gz -C /opt/
RUN ln -s /opt/proxima/bin/proxima /usr/bin/proxima
RUN ln -s /opt/proxima/bin/centauri /usr/bin/centauri

WORKDIR /workspace
ENTRYPOINT ["centauri"]
```

``` bash
# Собрать образ
docker build -t proxima-ide:1.0.0 .

# Запустить
docker run -it -v $(pwd):/workspace proxima-ide:1.0.0
```

### Kubernetes

``` yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: proxima-ide
spec:
  replicas: 3
  selector:
    matchLabels:
      app: proxima-ide
  template:
    metadata:
      labels:
        app: proxima-ide
    spec:
      containers:
      - name: ide
        image: proxima-ide:1.0.0
        ports:
        - containerPort: 8080
        resources:
          limits:
            memory: "4Gi"
            cpu: "2"
          requests:
            memory: "2Gi"
            cpu: "1"
```

## CI/CD

### GitHub Actions

См. .github/workflows/build.yml

### GitLab CI

``` yaml
stages:
  - build
  - test
  - deploy

build:
  stage: build
  script:
    - ./build.sh
  artifacts:
    paths:
      - build/

test:
  stage: test
  script:
    - cd build
    - ctest --output-on-failure

deploy:
  stage: deploy
  script:
    - ./deploy.sh
  only:
    - tags
```

## Устранение проблем

### Компилятор не находится

``` bash
# Проверить путь
which proxima

# Добавить в PATH
export PATH=$PATH:/usr/local/bin
```

### Qt не находится

``` bash
# Указать путь явно
export CMAKE_PREFIX_PATH=/opt/Qt5.15.2
```

### LLVM не находится

``` bash
# Найти LLVM
llvm-config --version

# Указать путь
export LLVM_DIR=/usr/lib/llvm-14/cmake
```

### Ошибки сборки

``` bash
# Очистить сборку
rm -rf build
mkdir build

# Собрать заново
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Проблемы с CUDA

``` bash
# Проверить CUDA
nvidia-smi
nvcc --version

# Указать путь к CUDA
export CUDA_HOME=/usr/local/cuda
export PATH=$PATH:$CUDA_HOME/bin
```

## Поддержка

Документация: https://prxlang.ru/docs
Issues: https://github.com/proxima-lang/proxima/issues
Email: dev@prxlang.ru
Discord: https://discord.gg/prxlang

## Proxima Development Team © 2026
