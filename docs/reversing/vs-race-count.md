# VS Race Count Notes

These notes apply to **MK8DX 4.0.0**.

## Visible Local VS values

The Race Count selector exposes:

```text
4, 6, 8, 12, 16, 24, 32, 48, 96
```

The selector maps its zero-based indexes to internal option IDs:

```text
[2, 3, 4, 5, 6, 7, 8, 9, 16]
```

`96 Races` is internal option ID `16` and was added in 4.0.0.

## Internal option conversion table

The internal conversion function accepts option IDs beyond the values shown in the Local VS selector:

| Option ID | Race count |
| ---: | ---: |
| 1 | 1 |
| 2 | 4 |
| 3 | 6 |
| 4 | 8 |
| 5 | 12 |
| 6 | 16 |
| 7 | 24 |
| 8 | 32 |
| 9 | 48 |
| 10 | 5 |
| 11 | 10 |
| 12 | 15 |
| 13 | 20 |
| 14 | 25 |
| 15 | 48 |
| 16 | 96 |

Option `15` exists internally but is **not** a selectable Local VS Race Count in 4.0.0. Do not present it as a user-facing No Limit option.

## Message IDs

```text
2  -> 0xA2A -> 4 Races
3  -> 0xA2C -> 6 Races
4  -> 0xA2D -> 8 Races
5  -> 0xA2E -> 12 Races
6  -> 0xA2F -> 16 Races
7  -> 0xA30 -> 24 Races
8  -> 0xA31 -> 32 Races
9  -> 0xA32 -> 48 Races
15 -> 0xA33 -> internal/special case
16 -> 0xA34 -> 96 Races
```

`0xA34` is absent from the base game data and present in update 4.0.0.

## Packed VS rules

Packed options live in a `uint32` at `+0x10`, with four bits per option. Packed slot 7 is Race Count.

A four-bit slot can represent only `0..15`, so option ID `16` cannot be represented by the legacy packed slot and must be handled by the newer 4.0.0 path.

## Relevant update offsets

| Symbol / purpose | Module offset |
| --- | ---: |
| Race Count selected-index to option ID | `0x44CB78` |
| Race Count option to value | `0x44CD40` |
| Race Count option text | `0x44CCC8` |
| Race Count row input handler | `0x44E660` |
| VS settings row configuration | `0x463BE4` |
| VS settings active-list range | `0x453E28` |

Additional addresses used by the mod are documented in `include/beerio/addresses.hpp`.
