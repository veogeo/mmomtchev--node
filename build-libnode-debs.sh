
#!/bin/bash
set -e

./configure --shared
make -j$(nproc)

# Ruta base del build de Node.js
BUILD_DIR="$(pwd)/out/Release"

# Verificar que libnode.so existe
if [ ! -f "$BUILD_DIR/libnode.so" ]; then
  echo "❌ libnode.so no encontrado en $BUILD_DIR"
  exit 1
fi

# Crear estructura de directorios para libnode109
mkdir -p debs/libnode109/usr/lib
mkdir -p debs/libnode109/DEBIAN

# Copiar libnode.so y symlinks
cp -a $BUILD_DIR/libnode.so* debs/libnode109/usr/lib/

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
# cp -r $BUILD_DIR/obj/gen/include/* debs/libnode-dev/usr/include/node/ || true
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

echo "✅ Paquetes generados:"
ls -lh debs/*.deb
