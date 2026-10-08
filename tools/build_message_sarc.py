#!/usr/bin/env python3
"""Add or update the Beerio popup message in MK8DX US English message.sarc.

The script expects the Mario Kart 8 Deluxe 4.0.0 US English archive and writes
an updated archive with MSBT label 60000. The input may be either an original
archive or one that was already patched by this script.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path

LABEL_NAME = "60000"
MENU_MSBT_NAME = "Menu.msbt"


class PatchError(RuntimeError):
    pass


def align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def u16(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def put_u32(data: bytearray, offset: int, value: int) -> None:
    struct.pack_into("<I", data, offset, value)


@dataclass
class SarcNode:
    index: int
    entry_offset: int
    name: str
    data_start: int
    data_end: int


def parse_sarc(data: bytes) -> tuple[int, list[SarcNode]]:
    if data[:4] != b"SARC":
        raise PatchError("Input is not a SARC archive.")
    if data[6:8] != b"\xff\xfe":
        raise PatchError("Only little-endian SARC archives are supported.")

    header_size = u16(data, 4)
    data_offset = u32(data, 0x0C)

    sfat = header_size
    if data[sfat : sfat + 4] != b"SFAT":
        raise PatchError("SARC is missing the SFAT section.")

    sfat_header_size = u16(data, sfat + 4)
    node_count = u16(data, sfat + 6)
    node_base = sfat + sfat_header_size
    sfnt = node_base + node_count * 0x10

    if data[sfnt : sfnt + 4] != b"SFNT":
        raise PatchError("SARC is missing the SFNT section.")

    sfnt_header_size = u16(data, sfnt + 4)
    name_base = sfnt + sfnt_header_size

    nodes: list[SarcNode] = []
    for index in range(node_count):
        entry = node_base + index * 0x10
        name_field = u32(data, entry + 4)
        if (name_field >> 24) == 0:
            name = ""
        else:
            name_offset = (name_field & 0x00FFFFFF) * 4
            name_start = name_base + name_offset
            name_end = data.find(b"\x00", name_start)
            if name_end < 0:
                raise PatchError("SARC contains an unterminated file name.")
            name = data[name_start:name_end].decode("utf-8")

        nodes.append(
            SarcNode(
                index=index,
                entry_offset=entry,
                name=name,
                data_start=u32(data, entry + 8),
                data_end=u32(data, entry + 12),
            )
        )

    return data_offset, nodes


@dataclass
class MsbtSection:
    magic: bytes
    data: bytes


def parse_msbt(data: bytes) -> tuple[bytearray, list[MsbtSection]]:
    if data[:8] != b"MsgStdBn":
        raise PatchError("Menu.msbt has an unexpected header.")
    if data[8:10] != b"\xff\xfe":
        raise PatchError("Only little-endian MSBT files are supported.")

    section_count = u16(data, 0x0E)
    sections: list[MsbtSection] = []
    offset = 0x20

    for _ in range(section_count):
        if offset + 0x10 > len(data):
            raise PatchError("MSBT section header is truncated.")
        magic = data[offset : offset + 4]
        size = u32(data, offset + 4)
        start = offset + 0x10
        end = start + size
        if end > len(data):
            raise PatchError("MSBT section data is truncated.")
        sections.append(MsbtSection(magic=magic, data=data[start:end]))
        offset = align(end, 0x10)

    return bytearray(data[:0x20]), sections


def parse_lbl1(data: bytes) -> tuple[int, list[list[tuple[str, int]]]]:
    if len(data) < 4:
        raise PatchError("LBL1 section is truncated.")
    group_count = u32(data, 0)
    table_end = 4 + group_count * 8
    if table_end > len(data):
        raise PatchError("LBL1 group table is truncated.")

    groups: list[list[tuple[str, int]]] = []
    for group_index in range(group_count):
        count = u32(data, 4 + group_index * 8)
        offset = u32(data, 8 + group_index * 8)
        cursor = offset
        labels: list[tuple[str, int]] = []

        for _ in range(count):
            if cursor >= len(data):
                raise PatchError("LBL1 label entry is truncated.")
            name_len = data[cursor]
            cursor += 1
            if cursor + name_len + 4 > len(data):
                raise PatchError("LBL1 label entry is truncated.")
            name = data[cursor : cursor + name_len].decode("utf-8")
            cursor += name_len
            message_index = u32(data, cursor)
            cursor += 4
            labels.append((name, message_index))

        groups.append(labels)

    return group_count, groups


def label_hash(label: str, group_count: int) -> int:
    value = 0
    for byte in label.encode("utf-8"):
        value = (value * 0x492 + byte) & 0xFFFFFFFF
    return value % group_count


def build_lbl1(groups: list[list[tuple[str, int]]]) -> bytes:
    group_count = len(groups)
    header_size = 4 + group_count * 8
    body = bytearray()
    offsets: list[int] = []

    for labels in groups:
        offsets.append(header_size + len(body))
        for name, message_index in labels:
            encoded_name = name.encode("utf-8")
            if len(encoded_name) > 0xFF:
                raise PatchError("MSBT label name is too long.")
            body.append(len(encoded_name))
            body.extend(encoded_name)
            body.extend(struct.pack("<I", message_index))

    out = bytearray(struct.pack("<I", group_count))
    for labels, offset in zip(groups, offsets):
        out.extend(struct.pack("<II", len(labels), offset))
    out.extend(body)
    return bytes(out)


def parse_txt2(data: bytes) -> list[bytes]:
    if len(data) < 4:
        raise PatchError("TXT2 section is truncated.")
    count = u32(data, 0)
    table_end = 4 + count * 4
    if table_end > len(data):
        raise PatchError("TXT2 offset table is truncated.")

    offsets = [u32(data, 4 + i * 4) for i in range(count)]
    messages: list[bytes] = []
    for index, start in enumerate(offsets):
        end = offsets[index + 1] if index + 1 < count else len(data)
        if start > end or end > len(data):
            raise PatchError("TXT2 contains an invalid message offset.")
        messages.append(data[start:end])
    return messages


def build_txt2(messages: list[bytes]) -> bytes:
    table_size = 4 + len(messages) * 4
    offsets: list[int] = []
    body = bytearray()

    for message in messages:
        offsets.append(table_size + len(body))
        body.extend(message)

    out = bytearray(struct.pack("<I", len(messages)))
    for offset in offsets:
        out.extend(struct.pack("<I", offset))
    out.extend(body)
    return bytes(out)


def patch_msbt(data: bytes, popup_text: str) -> bytes:
    header, sections = parse_msbt(data)

    try:
        lbl_index = next(i for i, section in enumerate(sections) if section.magic == b"LBL1")
        txt_index = next(i for i, section in enumerate(sections) if section.magic == b"TXT2")
    except StopIteration as exc:
        raise PatchError("Menu.msbt must contain LBL1 and TXT2 sections.") from exc

    group_count, groups = parse_lbl1(sections[lbl_index].data)
    messages = parse_txt2(sections[txt_index].data)

    message_index: int | None = None
    for labels in groups:
        for name, index in labels:
            if name == LABEL_NAME:
                message_index = index
                break
        if message_index is not None:
            break

    if message_index is None:
        message_index = len(messages)
        group = label_hash(LABEL_NAME, group_count)
        groups[group].append((LABEL_NAME, message_index))
        messages.append(b"")

    if message_index >= len(messages):
        raise PatchError(
            f"Label {LABEL_NAME} points to message {message_index}, "
            f"but TXT2 contains only {len(messages)} messages."
        )

    normalized = popup_text.replace("\r\n", "\n").replace("\r", "\n")
    normalized = normalized.rstrip("\n")
    messages[message_index] = normalized.encode("utf-16-le") + b"\x00\x00"

    sections[lbl_index] = MsbtSection(b"LBL1", build_lbl1(groups))
    sections[txt_index] = MsbtSection(b"TXT2", build_txt2(messages))

    out = bytearray(header)
    for section in sections:
        out.extend(section.magic)
        out.extend(struct.pack("<I", len(section.data)))
        out.extend(b"\x00" * 8)
        out.extend(section.data)
        out.extend(b"\xAB" * (align(len(out), 0x10) - len(out)))

    put_u32(out, 0x12, len(out))
    return bytes(out)


def patch_sarc(data: bytes, popup_text: str) -> bytes:
    data_offset, nodes = parse_sarc(data)
    menu = next((node for node in nodes if node.name == MENU_MSBT_NAME), None)
    if menu is None:
        raise PatchError("SARC does not contain Menu.msbt.")

    max_end = max(node.data_end for node in nodes)
    if menu.data_end != max_end:
        raise PatchError(
            "Menu.msbt is not the final SARC data file. "
            "This archive layout is not supported."
        )

    start = data_offset + menu.data_start
    end = data_offset + menu.data_end
    patched_msbt = patch_msbt(data[start:end], popup_text)

    out = bytearray(data[:start])
    out.extend(patched_msbt)

    put_u32(out, menu.entry_offset + 12, menu.data_start + len(patched_msbt))
    put_u32(out, 8, len(out))
    return bytes(out)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Add/update Beerio popup label 60000 in MK8DX 4.0.0 USen message.sarc."
    )
    parser.add_argument("--input", required=True, type=Path, help="Source message.sarc")
    parser.add_argument("--text", required=True, type=Path, help="UTF-8 popup text file")
    parser.add_argument("--output", required=True, type=Path, help="Patched message.sarc")
    args = parser.parse_args()

    try:
        source = args.input.read_bytes()
        popup_text = args.text.read_text(encoding="utf-8")
        patched = patch_sarc(source, popup_text)
    except (OSError, UnicodeError, PatchError) as exc:
        parser.error(str(exc))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(patched)
    print(f"Wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
