# MK8DX Beerio Reminder

A Mario Kart 8 Deluxe mod that adds configurable Beerio reminders to **Local VS Race**.

Before selected races begin, the mod opens a native Yes/No confirmation dialog. Choosing **No** returns to the pre-race screen and keeps that race due for a reminder. Choosing **Yes** closes the dialog and starts the race.

## Features

- Adds **Beerio Race Frequency** to the Local VS settings menu.
- Adds **Beerio Race Start** to the Local VS settings menu.
- Both settings automatically use the selected **Race Count** as their maximum.
- Supports Race Counts up to **96**.
- Prompts before the configured race, not after it.
- Uses Mario Kart 8 Deluxe's native Yes/No dialog.

Example: with **Frequency = 4** and **Start = 3**, reminders appear before races **3, 7, 11, 15, ...**.

## Compatibility

| Item | Supported value |
| --- | --- |
| Game | Mario Kart 8 Deluxe |
| Game version | **4.0.0** |
| Base title ID | `0100152000022000` |
| Update title ID | `0100152000022800` |
| Update version | `v1441792` |
| Main Build ID | `2c336a9bcf79c3040ce506cdd391b57800000000000000000000000000000000` |
| Mode | Local VS Race |
| CFW | Atmosphère |
| Popup language resource | US English (`USen`) |

Other game versions are not supported unless their offsets are re-verified and updated.

## Installation

Download the release package and copy its `atmosphere` folder to the root of the SD card, then fully close and relaunch Mario Kart 8 Deluxe.

The installed files live under:

```text
atmosphere/contents/0100152000022000/
├── exefs/
│   ├── main.npdm
│   └── subsdk9
└── romfs/
    └── UI/USen/message.sarc
```

## Using the settings

**Beerio Race Frequency** controls how often the popup appears. For example, `Every 4 Races` means every fourth race after the configured start.

**Beerio Race Start** controls the first race that receives the popup. For example, `3rd Race` starts the sequence at race 3.

Both values are limited to the currently selected Race Count.

## Customization

The popup text and the two added VS setting labels are intentionally easy to change. See [docs/CUSTOMIZATION.md](docs/CUSTOMIZATION.md).

The source repository does **not** need to contain an extracted Nintendo `message.sarc`. The included tool can build the required popup resource from your own Mario Kart 8 Deluxe 4.0.0 US English archive.

## Building from source

See [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) for the devkitPro setup, build commands, optional FTP deployment, and resource generation workflow.

Clone with submodules:

```bash
git clone --recursive <repository-url>
```

If the repository was already cloned without submodules:

```bash
git submodule update --init --recursive
```

## Known issue

After choosing **Yes**, the previous menu's yellow selection highlight can briefly appear in the lower-right area while the native race-start action is replayed. This is cosmetic and does not affect the race or reminder logic.

## Reverse-engineering notes

Version and VS Race Count research is kept in [docs/reversing](docs/reversing/).

Release packaging notes are in [docs/PUBLISHING.md](docs/PUBLISHING.md).

## Responsible use

The reminder can be customized for any activity or non-alcoholic drink. If alcohol is involved, follow applicable laws and your own limits; never drink and drive.

## License

This project is distributed under the **GNU General Public License v2.0**. See [LICENSE](LICENSE). Third-party/submodule code may have its own license terms.
