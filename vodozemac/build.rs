fn main() {
    let mut build = cxx_build::bridge("src/lib.rs");
    build
        .file("src/lib.cpp")
        .std("c++20")
        .static_flag(true)
        .include("include");

    if std::env::var("TARGET") == Ok(String::from("wasm32-unknown-emscripten")) {
        build.flag("-fexceptions").flag("-sNO_DISABLE_EXCEPTION_CATCHING").compile("libvodozemac.a");
    } else {
        build.compile("libvodozemac.a");
    }

    println!("cargo:rerun-if-changed=src/lib.cpp");
    println!("cargo:rerun-if-changed=include/vodozemac.h");
}
