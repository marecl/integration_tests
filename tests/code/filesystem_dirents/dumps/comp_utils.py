from dataclasses import dataclass
import hashlib
import re


find_buffer_end_re = re.compile(b"\x00+(\xaa+)")
regular_dirent_query_re = re.compile(
    b"(?P<fileno>....)(?P<reclen>..)(?P<entry_type>[\x02\x04\x08])(?P<namelen>.)(?P<name>[ -~]{1,255})(?P<padding>\x00*)"
)
pfs_query_getdents_re = re.compile(
    b"(?P<fileno>.{4})(?P<entry_type>[\x02\x04\x08]\x00{3})(?P<namelen>....)(?P<reclen>....)(?P<name>[ -~]{1,255})(?P<padding>\x00*)"
)

@dataclass(kw_only=True)
class Dirent:
    chk: bytes
    offset: int
    end: int
    entry_type: bytes  # 4 for pfs, 1 for reg
    fileno: bytes  # 4 for pfs, 4 for reg
    namelen: bytes  # 4 for pfs, 1 for reg
    reclen: bytes  # 4 for pfs, 2 for reg
    name: bytes  # up to 255 characters + null terminator
    padding: bytes

    def is_dirent(self):
        pass

    def hash(self) -> bytes:
        if not self.chk:
            self.chk = hashlib.md5(
                self.entry_type + self.namelen + self.reclen + self.name
            ).digest()
        return self.chk

    def __repr__(self) -> bytes:
        return self.hash()

    def __eq__(self, other):
        if isinstance(other, Dirent):
            return False
        return self.hash() == other.hash()