# VS Race Count

## MK8DX 4.0.0

### Count option conversion

Update function:
710044cd40

Base equivalent:
6037a4ac

Base accepts option IDs 1..15.
Update accepts option IDs 1..16.

Mapping:

| Option | Value |
|---:|---:|
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

Option 16 was added in update 4.0.0.

### Race Count messages

2  -> 0xA2A -> 4 Races
3  -> 0xA2C -> 6 Races
4  -> 0xA2D -> 8 Races
5  -> 0xA2E -> 12 Races
6  -> 0xA2F -> 16 Races
7  -> 0xA30 -> 24 Races
8  -> 0xA31 -> 32 Races
9  -> 0xA32 -> 48 Races
15 -> 0xA33 -> No Limit
16 -> 0xA34 -> 96 Races

0xA34 is absent from base and present in update 4.0.0.

### Packed VS rules

Packed options live in a uint32 at +0x10.
Each option occupies four bits.

Packed slot 7 is Race Count.

Because four bits can only encode 0..15,
option 16 cannot be represented by the legacy packed slot.

### addresses

getRaceSettings
    71004245d4

copyRaceSettings
    7100424130

loadRaceSettings
    710042406c

countOptionToValue
    710044cd40

getRaceCountOptionText
    710044ccc8

getPackedRuleOption
    71008488b8

applyRaceSettings
    7100501008

RaceCountMessageIds
    7100f273e0

RaceCountMessageId96
    7100f27418

CountOptionValues
    7100f27460

RaceSettings::RaceCountOption
    +0x38

ActiveSession::ResolvedRaceCount
    +0x1438
	
### Runtime validation — MK8DX 4.0.0

countOptionToValue:
Ghidra VA:      0x710044CD40
module offset:  0x44CD40

Runtime test:
Selecting "96 Races" caused the hook to execute with w1 = 16.

Therefore:
RaceCountOption 16 = 96 Races

Note:
Option 15 exists in the internal conversion table and is special-cased by
other game logic, but it is not a selectable VS Race option in MK8DX 4.0.0.
Do not currently identify it as a user-facing "No Limit" option.