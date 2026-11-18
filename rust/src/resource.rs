//! Validated record wrapper and embedded PNG parser for the supplied J2ME format.
//! Numbered record semantics beyond this container are not yet reverse mapped.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ResourceError {
    TooShort, InvalidLength { expected: usize, actual: usize }, CorruptPng
}
fn be32(bytes: &[u8]) -> Result<u32, ResourceError> {
    if bytes.len() < 4 { return Err(ResourceError::TooShort) }
    Ok(u32::from_be_bytes([bytes[0],bytes[1],bytes[2],bytes[3]]))
}
pub fn unpack_record(data: &[u8]) -> Result<&[u8], ResourceError> {
    let header = be32(data)?;
    let expected = (header & 0x3fff_ffff) as usize;
    let actual = data.len()-4;
    if (header & 0xc000_0000) != 0x4000_0000 || actual != expected {
        return Err(ResourceError::InvalidLength{expected,actual});
    }
    Ok(&data[4..])
}
pub fn find_png_bytes(data: &[u8]) -> Result<Option<&[u8]>, ResourceError> {
    const MAGIC: &[u8] = b"\x89PNG\r\n\x1a\n";
    let start = match data.windows(MAGIC.len()).position(|x| x==MAGIC) {
        Some(index)=>index, None=>return Ok(None)
    };
    let mut at=start+MAGIC.len();
    for _ in 0..500 {
        if at.checked_add(12).map_or(true,|end|end>data.len()) {
            return Err(ResourceError::CorruptPng);
        }
        let len=be32(&data[at..])? as usize;
        let end=at.checked_add(12).and_then(|v|v.checked_add(len))
            .ok_or(ResourceError::CorruptPng)?;
        if end>data.len(){ return Err(ResourceError::CorruptPng) }
        let kind=&data[at+4..at+8];
        at=end;
        if kind==b"IEND" { return Ok(Some(&data[start..at])); }
    }
    Err(ResourceError::CorruptPng)
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test] fn valid_wrapper() {
        assert_eq!(unpack_record(&[0x40,0,0,3,1,2,3]),Ok(&[1,2,3][..]));
    }
    #[test] fn bad_wrapper() {
        assert!(unpack_record(&[0x40,0,0,4,1,2,3]).is_err());
    }
    #[test] fn short_wrapper() {
        assert_eq!(unpack_record(&[1,2]), Err(ResourceError::TooShort));
    }
}
