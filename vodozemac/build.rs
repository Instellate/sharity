use std::error::Error;
use std::path::Path;

fn main() -> Result<(), Box<dyn Error>> {
    let mut build = cxx_build::bridge("src/lib.rs");
    build
        .file("src/lib.cpp")
        .cpp(true)
        .std("c++20")
        .static_flag(true)
        .include("include");

    let ffi_header = Path::new("target/cxxbridge/vodozemac-cpp/src/lib.rs.h");
    let output_dir = Path::new("include/vodozemac/internals");
    if !output_dir.exists() {
        std::fs::create_dir_all(output_dir)?;
    }
    std::fs::copy(ffi_header, output_dir.join("ffi.h"))?;

    let target_string = std::env::var("TARGET");
    let target = target_string.as_ref().map(|v| v.as_str());

    if target == Ok("wasm32-unknown-emscripten") {
        build
            .flag("-fexceptions")
            .flag("-sNO_DISABLE_EXCEPTION_CATCHING")
            .compile("libvodozemac.a");
    } else if target == Ok("x86_64-pc-wiindows-msvc") {
        println!("cargo::rustc-link-lib=bcrypt");
        build.compile("libvodozemac.lib");
    } else {
        build.compile("libvodozemac.a");
    }

    println!("cargo:rerun-if-changed=src/lib.cpp");
    println!("cargo:rerun-if-changed=include/vodozemac.h");

    Ok(())
}
