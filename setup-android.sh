#!/bin/bash
# Running this script requires specifiying Android SDK, NDK and Qt Android installations
# Pass environment variables ANDROID_SDK, ANDROID_NDK, ANDROID_QT
set -euo pipefail
IFS=$'\n\t'

if [[ -z "${ANDROID_QT+x}" ]]; then
    echo "Variable ANDROID_QT is not set"
    exit 1
fi

if [[ ! -x "${ANDROID_QT}/bin/qt-cmake" ]]; then
    echo "${ANDROID_QT} is not a valid Qt installation"
    exit 1
fi

if [[ -z "${ANDROID_SDK+x}" ]]; then
    echo "ANDROID_SDK is not set"
    exit 1
fi

if [[ -z "${ANDROID_NDK+x}" ]]; then
    echo "ANDROID_NDK is not set"
    exit 1
fi

if [[ ! "${ANDROID_NDK}" == "${ANDROID_SDK}"* ]]; then
    echo "The NDK ${ANDROID_NDK} is not a part of the ${ANDROID_SDK} SDK"
fi

project_root="$(pwd)"
android_libs="${project_root}/android-libraries"
lib_install="${android_libs}/install"

sharity_client="${project_root}/client"
vodozemac="${project_root}/vodozemac"

if [[ -z "${SKIP_LIBRARIES+x}" ]]; then
  mkdir "${android_libs}"
  mkdir "${lib_install}"

  cd "${android_libs}"

  # Build OpenSSL
  git clone https://github.com/openssl/openssl --depth 1 -b openssl-3.6.0
  cd openssl

  export ANDROID_NDK_ROOT="${ANDROID_NDK}"
  openssl_path="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH"
  PATH="${openssl_path}" ./Configure android-arm64 -D__ANDROID_API__=21 --prefix=/
  PATH="${openssl_path}" make -j$(nproc)
  PATH="${openssl_path}" make DESTDIR="${lib_install}" install_sw install_ssldirs

  # Build LibDataChannel
  cd "${android_libs}"
  git clone https://github.com/paullouisageneau/libdatachannel --depth 1 --recursive -b v0.24.1

  cd libdatachannel
  cmake -B build -GNinja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_ANDROID_NDK="${ANDROID_NDK}" \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DOPENSSL_SSL_LIBRARY="${lib_install}/lib/libssl.so" \
      -DOPENSSL_CRYPTO_LIBRARY="${lib_install}/lib/libcrypto.so" \
      -DOPENSSL_INCLUDE_DIR="${lib_install}/include" \
      -DNO_EXAMPLES=ON
  cmake --build build
  cmake --install build --prefix "${lib_install}"
fi

# Get necessary environment variables to build Vodozemac bindings properly
cd "${vodozemac}"
cargo ndk-env --target aarch64-linux-android > "${sharity_client}/android-env"

# Create configure-android.sh
cd "${sharity_client}"

configure_cmake_options="-DANDROID_SDK_ROOT=\"\${android_sdk}\" \\
    -DANDROID_NDK_ROOT=\"\${android_ndk}\" \\
    -DQT_ANDROID_ABIS=\"arm64-v8a\" \\
    -DLibDataChannel_DIR=\"${lib_install}/lib/cmake/LibDataChannel\""

configure_build_release="${configure_cmake_options} \\
    -DCMAKE_BUILD_TYPE=Release"

if [[ -n "${BUILD_RELEASE+x}" ]]; then
  configure_cmake_options="${configure_build_release}"
else
  read -r -p "Build for release [y/N]: " build_release
  case "${build_release}" in
      [Yy]*) configure_cmake_options="${configure_build_release}" ;;
      *) ;;
  esac
fi

if [[ -n "${ANDROID_SIGN+x}" ]]; then
  configure_cmake_options="${configure_cmake_options} \\
    -DQT_ANDROID_SIGN_APK=ON"
fi

configure_cmake="${ANDROID_QT}/bin/qt-cmake \\
    -B build-android \\
    -S ${sharity_client} \\
    -GNinja \\
    ${configure_cmake_options}"

echo "#!/bin/sh
android_sdk=\"${ANDROID_SDK}\"
android_ndk=\"${ANDROID_NDK}\"
source \"${sharity_client}/android-env\"
${configure_cmake}" > "${sharity_client}/configure-android.sh"
chmod +x "${sharity_client}/configure-android.sh"

echo 'Successfully setup everything for android'
echo 'Run "client/configure-android.sh" to configure CMake for android'
echo 'Before building remember to run "source client/android-env"'
echo 'Run "cmake --build build-android --target apk" to build an APK'
echo 'Run "cmake --build build-android --target sharity-client" to build the client without an APK'

