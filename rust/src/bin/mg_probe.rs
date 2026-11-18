use mg_legacy_core::resource::unpack_record;
use std::{env, fs, path::Path};

fn main() {
    let Some(folder) = env::args().nth(1) else {
        eprintln!("Usage: mg_probe <locally extracted content/legacy_raw>");
        std::process::exit(2);
    };
    let mut count=0usize;
    let mut bytes=0usize;
    for n in 1..=74 {
        let path=Path::new(&folder).join(format!("f{n}"));
        let original=fs::read(&path).unwrap_or_else(|e| panic!("{}: {e}",path.display()));
        let payload=unpack_record(&original)
            .unwrap_or_else(|e| panic!("{}: {e:?}",path.display()));
        count+=1;
        bytes+=payload.len();
    }
    println!("{count} validated records; {bytes} payload bytes");
}
