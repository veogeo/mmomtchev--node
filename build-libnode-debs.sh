#!/bin/bash
set -e

# Variables
PYTHON_VERSION="3.11.9"
PYENV_ROOT="$HOME/.pyenv"
BUILD_DIR="$(pwd)/out/Release"

# Instalar pyenv si no existe
if [ ! -d "$PYENV_ROOT" ]; then
  curl https://pyenv.run | bash
fi

# Cargar pyenv en este script
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"

# Instalar Python 3.11.9 si no está
if ! pyenv versions --bare | grep -q "^${PYTHON_VERSION}$"; then
  pyenv install "$PYTHON_VERSION"
fi

# Activar temporalmente Python 3.11.9 solo para este script
pyenv shell "$PYTHON_VERSION"

# Confirmación
echo "✅ Python en uso: $(python --version)"

# Configurar y compilar Node.js
./configure --shared
make -j"$(nproc)"

# Verificar que libnode.so existe
if [ ! -f "$BUILD_DIR/libnode.so" ]; then
  echo "❌ libnode.so no encontrado en $BUILD_DIR"
  exit 1
fi

# Crear estructura de directorios para libnode109
mkdir -p debs/libnode109/usr/lib
mkdir -p debs/libnode109/DEBIAN

# Copiar libnode.so y symlinks
cp -a "$BUILD_DIR/libnode.so"* debs/libnode109/usr/lib/

# Crear control file para libnode109
cat <<EOF > debs/libnode109/DEBIAN/control
Package: libnode109
Version: 18.19.1+dfsg-6ubuntu5
Section: libs
Priority: optional
Architecture: amd64
Maintainer: César Benjamín <cesarbenjamindotnet@gmail.com>
Description: Node.js 18 libnode.so con soporte de embedding y napi_create_environment
EOF

# Crear estructura de directorios para libnode-dev
mkdir -p debs/libnode-dev/usr/include/node
mkdir -p debs/libnode-dev/DEBIAN

# Copiar headers
cp -r src/*.h debs/libnode-dev/usr/include/node/ || true
cp -r deps/v8/include/* debs/libnode-dev/usr/include/node/
cp config.gypi debs/libnode-dev/usr/include/node/ || true

# Crear control file para libnode-dev
cat <<EOF > debs/libnode-dev/DEBIAN/control
Package: libnode-dev
Version: 18.19.1
Section: libdevel
Priority: optional
Architecture: amd64
Maintainer: César Benjamín <cesarbenjamindotnet@gmail.com>
Depends: libnode109 (=18.19.1+dfsg-6ubuntu5)
Description: Headers de desarrollo para Node.js libnode.so
EOF

# Crear los .deb
dpkg-deb --build debs/libnode109
dpkg-deb --build debs/libnode-dev

# Limpiar pyenv y Python
pyenv shell --unset || true
rm -rf "$PYENV_ROOT"

echo "✅ Paquetes generados:"
ls -lh debs/*.deb
